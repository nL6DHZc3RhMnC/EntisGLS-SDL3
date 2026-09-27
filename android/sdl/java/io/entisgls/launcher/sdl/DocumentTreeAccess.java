package io.entisgls.launcher.sdl;

import android.content.ContentResolver;
import android.content.Context;
import android.content.UriPermission;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.system.ErrnoException;
import android.system.Os;
import android.system.OsConstants;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.security.MessageDigest;
import java.util.Properties;
import java.util.UUID;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Locale;

/** JNI-facing filesystem for one user-authorized document tree. Never turns a URI into a raw path. */
public final class DocumentTreeAccess {
    private static final String[] COLUMNS = {
        DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_DISPLAY_NAME,
        DocumentsContract.Document.COLUMN_MIME_TYPE, DocumentsContract.Document.COLUMN_SIZE,
        DocumentsContract.Document.COLUMN_LAST_MODIFIED, DocumentsContract.Document.COLUMN_FLAGS
    };
    private final ContentResolver resolver;
    private final Uri tree;
    private final String rootId;
    private final File replacementJournal;

    private static final class Entry {
        final String id, name;
        final boolean directory;
        final long size, modified, flags;
        Entry(Cursor cursor) throws IOException {
            id = cursor.getString(0);
            name = cursor.getString(1);
            directory = DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(2));
            size = directory ? 0 : (cursor.isNull(3) ? -1 : cursor.getLong(3));
            modified = cursor.isNull(4) ? 0 : cursor.getLong(4);
            flags = cursor.isNull(5) ? 0 : cursor.getLong(5);
            if (id == null || id.isEmpty() || id.length() > 65536) throw new IOException("文件提供方返回了无效标识。");
        }
        boolean writable() {
            return (flags & (directory ? DocumentsContract.Document.FLAG_DIR_SUPPORTS_CREATE : DocumentsContract.Document.FLAG_SUPPORTS_WRITE)) != 0;
        }
    }

    public DocumentTreeAccess(Context context, Uri selectedTree) throws IOException {
        resolver = context.getApplicationContext().getContentResolver();
        tree = selectedTree;
        try {
            if (tree == null || !"content".equals(tree.getScheme()) || !DocumentsContract.isTreeUri(tree))
                throw new IOException("请选择游戏文件夹。");
            rootId = DocumentsContract.getTreeDocumentId(tree);
            File journals = new File(context.getFilesDir(), "document-transactions");
            if (!journals.isDirectory() && !journals.mkdirs()) throw new IOException("无法创建存档事务目录。");
            replacementJournal = new File(journals, treeFingerprint(tree.toString()) + ".properties");
            boolean authorized = false;
            for (UriPermission grant : resolver.getPersistedUriPermissions())
                if (tree.equals(grant.getUri()) && grant.isReadPermission() && grant.isWritePermission()) authorized = true;
            if (!authorized) throw new IOException("所选游戏文件夹的读写授权已失效，请重新选择同一文件夹。");
            Entry root = byId(rootId);
            if (root == null || !root.directory || !root.writable())
                throw new IOException("所选文件夹不可读写，请选择可写的本地游戏文件夹。");
            recoverReplacement();
        } catch (SecurityException | IllegalArgumentException error) {
            throw accessError(error);
        }
    }

    private static String treeFingerprint(String value) throws IOException {
        try {
            byte[] hash = MessageDigest.getInstance("SHA-256").digest(value.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            StringBuilder text = new StringBuilder();
            for (byte item : hash) text.append(String.format(Locale.ROOT, "%02x", item & 255));
            return text.toString();
        } catch (java.security.NoSuchAlgorithmException error) { throw new IOException(error); }
    }

    private void writeReplacement(String source, String target, String backup) throws IOException {
        Properties transaction = new Properties();
        transaction.setProperty("tree", tree.toString());
        transaction.setProperty("source", source);
        transaction.setProperty("target", target);
        transaction.setProperty("backup", backup);
        File temporary = new File(replacementJournal.getPath() + ".tmp");
        try (FileOutputStream output = new FileOutputStream(temporary)) {
            transaction.store(output, "EntisGLS document replacement v1");
            output.getFD().sync();
        }
        try { Os.rename(temporary.getAbsolutePath(), replacementJournal.getAbsolutePath()); }
        catch (ErrnoException error) { throw new IOException("无法保存存档事务记录。", error); }
    }
    private void clearReplacement() throws IOException {
        if (replacementJournal.exists() && !replacementJournal.delete()) throw new IOException("无法清理已完成的存档事务记录。");
    }
    private void renameEntry(Entry entry, String newName) throws IOException {
        if (DocumentsContract.renameDocument(resolver, uri(entry.id), newName) == null)
            throw new IOException("文件提供方不支持重命名此资源。");
    }

    /** Recover a process death between provider operations without deleting an inferred old save. */
    private void recoverReplacement() throws IOException {
        if (!replacementJournal.isFile()) return;
        if (replacementJournal.length() > 32768) throw new IOException("存档事务记录无效，已保留现有文件。");
        Properties transaction = new Properties();
        try (FileInputStream input = new FileInputStream(replacementJournal)) { transaction.load(input); }
        String source = checkedPath(transaction.getProperty("source"));
        String target = checkedPath(transaction.getProperty("target"));
        String backup = checkedPath(transaction.getProperty("backup"));
        if (!tree.toString().equals(transaction.getProperty("tree")) || target.isEmpty() || source.isEmpty()
                || !parent(source).equals(parent(target)) || !parent(backup).equals(parent(target))
                || !name(backup).matches("\\.entis-save-backup-[0-9a-f-]{36}"))
            throw new IOException("存档事务记录无效，已保留现有文件。");
        Entry saved = resolve(backup);
        Entry destination = resolve(target);
        if (saved != null) {
            if (destination == null) {
                renameEntry(saved, name(target));
            } else {
                // The replacement may have completed just before process death.
                // Keep its predecessor instead of guessing which file the user wants.
                android.util.Log.w("EntisGLS", "Interrupted save preserved backup: " + backup, null);
            }
        } else if (destination == null) {
            throw new IOException("存档事务尚未恢复，目标与备份均不可见。请检查存储设备，已有文件未删除。");
        }
        clearReplacement();
    }

    static String checkedPath(String path) throws IOException {
        if (path == null || path.length() > 4096 || path.indexOf('\\') >= 0 || path.indexOf('\0') >= 0)
            throw new IOException("游戏资源路径无效。");
        if (path.isEmpty()) return path;
        String[] parts = path.split("/", -1);
        if (parts.length > 64) throw new IOException("游戏资源目录层级过深。");
        for (String part : parts) if (part.isEmpty() || part.equals(".") || part.equals(".."))
            throw new IOException("游戏资源路径不能越出所选文件夹。");
        return path;
    }

    private static String parent(String path) { int slash = path.lastIndexOf('/'); return slash < 0 ? "" : path.substring(0, slash); }
    private static String name(String path) { return path.substring(path.lastIndexOf('/') + 1); }
    private Uri uri(String id) { return DocumentsContract.buildDocumentUriUsingTree(tree, id); }
    private static IOException accessError(Exception cause) { return new IOException("无法访问所选游戏文件夹，请检查连接与读写授权，必要时重新选择同一文件夹。", cause); }
    private Entry byId(String id) throws IOException { return byUri(uri(id)); }
    private Entry byUri(Uri document) throws IOException {
        try (Cursor cursor = resolver.query(document, COLUMNS, null, null, null)) {
            if (cursor == null) throw new IOException("文件提供方无法读取资源信息。");
            return cursor.moveToFirst() ? new Entry(cursor) : null;
        }
    }
    private ArrayList<Entry> children(Entry directory) throws IOException {
        if (directory == null || !directory.directory) throw new IOException("资源目录不存在。");
        ArrayList<Entry> entries = new ArrayList<>();
        HashSet<String> names = new HashSet<>();
        try (Cursor cursor = resolver.query(DocumentsContract.buildChildDocumentsUriUsingTree(tree, directory.id), COLUMNS, null, null, null)) {
            if (cursor == null) throw new IOException("无法列出所选游戏文件夹。");
            while (cursor.moveToNext()) {
                if (Thread.currentThread().isInterrupted()) throw new IOException("资源检查已取消。");
                Entry entry = new Entry(cursor);
                checkedPath(entry.name);
                if (entry.name.isEmpty() || entry.name.indexOf('/') >= 0) throw new IOException("文件提供方返回了无效文件名。");
                if (!names.add(entry.name.toLowerCase(Locale.ROOT))) throw new IOException("发现仅大小写不同的重复资源：" + entry.name);
                entries.add(entry);
                if (entries.size() > 200000) throw new IOException("所选文件夹文件过多。");
            }
        }
        return entries;
    }
    private Entry resolve(String path) throws IOException {
        checkedPath(path);
        Entry current = byId(rootId);
        if (path.isEmpty()) return current;
        HashSet<String> visited = new HashSet<>();
        visited.add(rootId);
        for (String part : path.split("/")) {
            if (current == null || !current.directory) return null;
            Entry found = null;
            for (Entry entry : children(current)) if (entry.name.equalsIgnoreCase(part)) {
                if (found != null) throw new IOException("发现大小写不明确的资源路径：" + path);
                found = entry;
            }
            current = found;
            if (current != null && !visited.add(current.id)) throw new IOException("文件提供方返回了循环目录。");
        }
        return current;
    }

    /** kind: 0 missing, 1 file, 2 directory; size; modification milliseconds; writable. */
    public synchronized long[] stat(String relative) throws IOException {
        try {
            recoverReplacement();
            Entry entry = resolve(relative);
            if (entry == null) return new long[] {0, 0, 0, 0};
            long size = entry.size;
            if (size < 0) {
                try (ParcelFileDescriptor descriptor = resolver.openFileDescriptor(uri(entry.id), "r")) {
                    if (descriptor == null) throw new IOException("无法查询游戏资源大小：" + relative);
                    try { size = Os.lseek(descriptor.getFileDescriptor(), 0, OsConstants.SEEK_END); }
                    catch (ErrnoException error) { throw new IOException("文件提供方无法报告资源大小或不支持随机读取：" + relative, error); }
                }
            }
            return new long[] {entry.directory ? 2 : 1, size, entry.modified, entry.writable() ? 1 : 0};
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }
    public synchronized String[] list(String relative) throws IOException {
        try {
            recoverReplacement();
            ArrayList<Entry> entries = children(resolve(relative));
            String[] names = new String[entries.size()];
            for (int i = 0; i < entries.size(); ++i) names[i] = entries.get(i).name;
            return names;
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }
    public synchronized String displayName() throws IOException {
        try { Entry root = byId(rootId); return root == null || root.name == null || root.name.trim().isEmpty() ? "游戏文件夹" : root.name; }
        catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }

    /** Ownership of the seekable descriptor transfers to native code, which must close it. */
    public synchronized int open(String relative, String mode) throws IOException {
        Uri created = null;
        boolean transferred = false;
        try {
            recoverReplacement();
            checkedPath(relative);
            if (relative.isEmpty()) throw new IOException("无法将游戏根目录作为文件打开。");
            if (!"r".equals(mode) && !"rw".equals(mode) && !"rwt".equals(mode) && !"wa".equals(mode))
                throw new IOException("不支持的资源打开模式。");
            boolean write = !"r".equals(mode);
            Entry entry = resolve(relative);
            Uri document;
            if (entry == null) {
                if (!write || "rw".equals(mode)) throw new java.io.FileNotFoundException(relative);
                Entry directory = resolve(parent(relative));
                if (directory == null || !directory.directory || !directory.writable()) throw new IOException("资源所在目录不可写。");
                created = DocumentsContract.createDocument(resolver, uri(directory.id), "application/octet-stream", name(relative));
                if (created == null) throw new IOException("无法在所选游戏文件夹创建文件。");
                Entry actual = byUri(created);
                if (actual == null || actual.directory || !name(relative).equals(actual.name))
                    throw new IOException("文件提供方改变了文件名，无法保存游戏资源。");
                document = created;
            } else {
                if (entry.directory || (write && !entry.writable())) throw new IOException("资源文件不可按要求读写：" + relative);
                document = uri(entry.id);
            }
            try (ParcelFileDescriptor descriptor = resolver.openFileDescriptor(document, mode)) {
                if (descriptor == null) throw new IOException("无法打开游戏资源：" + relative);
                try { Os.lseek(descriptor.getFileDescriptor(), 0, OsConstants.SEEK_CUR); }
                catch (ErrnoException error) { throw new IOException("所选文件提供方不支持随机读写，请将游戏放在手机本地存储或可随机访问的存储设备。", error); }
                int fd = descriptor.detachFd();
                transferred = true;
                return fd;
            }
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
        finally {
            if (created != null && !transferred) {
                try { DocumentsContract.deleteDocument(resolver, created); }
                catch (Exception ignored) { /* Only our newly created file may be cleaned up. */ }
            }
        }
    }

    public synchronized void mkdir(String relative) throws IOException {
        try {
            recoverReplacement();
            checkedPath(relative);
            if (relative.isEmpty()) return;
            String path = "";
            for (String part : relative.split("/")) {
                String next = path.isEmpty() ? part : path + "/" + part;
                Entry existing = resolve(next);
                if (existing != null) {
                    if (!existing.directory) throw new IOException("目录名已被文件占用：" + next);
                } else {
                    Entry directory = resolve(path);
                    if (directory == null || !directory.writable()) throw new IOException("资源目录不可写：" + path);
                    Uri made = DocumentsContract.createDocument(resolver, uri(directory.id), DocumentsContract.Document.MIME_TYPE_DIR, part);
                    if (made == null) throw new IOException("无法创建游戏目录：" + next);
                    Entry actual = byUri(made);
                    if (actual == null || !actual.directory || !part.equals(actual.name)) {
                        // The requested path would point somewhere else. Remove
                        // only this freshly-created empty directory.
                        try { DocumentsContract.deleteDocument(resolver, made); } catch (Exception ignored) {}
                        throw new IOException("文件提供方改变了目录名，无法保存游戏资源。");
                    }
                }
                path = next;
            }
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }
    public synchronized void remove(String relative, boolean directory) throws IOException {
        try {
            recoverReplacement();
            checkedPath(relative);
            if (relative.isEmpty()) throw new IOException("不能删除所选游戏根目录。");
            Entry entry = resolve(relative);
            if (entry == null) throw new java.io.FileNotFoundException(relative);
            if (entry.directory != directory) throw new IOException("资源类型与删除操作不符。");
            if (directory && !children(entry).isEmpty()) throw new IOException("只能删除空目录。");
            if (!DocumentsContract.deleteDocument(resolver, uri(entry.id))) throw new IOException("无法删除资源：" + relative);
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }
    public synchronized void rename(String oldRelative, String newRelative) throws IOException {
        try {
            recoverReplacement();
            checkedPath(oldRelative); checkedPath(newRelative);
            if (oldRelative.isEmpty() || newRelative.isEmpty()) throw new IOException("不能重命名所选游戏根目录。");
            if (!parent(oldRelative).equals(parent(newRelative))) throw new IOException("此文件访问方式暂不支持跨目录移动。");
            Entry source = resolve(oldRelative);
            if (source == null) throw new java.io.FileNotFoundException(oldRelative);
            Entry target = resolve(newRelative);
            if (oldRelative.equals(newRelative)) return;
            if (target == null || target.id.equals(source.id)) {
                renameEntry(source, name(newRelative));
                return;
            }
            if (source.directory || target.directory) throw new IOException("不能以文件替换目录或覆盖已有目录。");
            String backupName = ".entis-save-backup-" + UUID.randomUUID();
            String backupPath = parent(newRelative).isEmpty() ? backupName : parent(newRelative) + "/" + backupName;
            if (resolve(backupPath) != null) throw new IOException("存档备份文件名冲突，请重试。");
            // Save the recovery record before moving either provider document.
            writeReplacement(oldRelative, newRelative, backupPath);
            try {
                renameEntry(target, backupName);
                renameEntry(source, name(newRelative));
            } catch (IOException | RuntimeException failure) {
                try { recoverReplacement(); } catch (IOException recovery) { failure.addSuppressed(recovery); }
                throw failure;
            }
            // Publication has succeeded. Failure to remove a backup must never
            // turn a completed save into data loss, and recovery keeps it too.
            Entry backup = resolve(backupPath);
            if (backup != null) {
                try { DocumentsContract.deleteDocument(resolver, uri(backup.id)); }
                catch (Exception ignored) { android.util.Log.w("EntisGLS", "Previous save backup retained: " + backupPath, null); }
            }
            clearReplacement();
        } catch (SecurityException | IllegalArgumentException error) { throw accessError(error); }
    }
}
