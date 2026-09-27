package io.entisgls.launcher.sdl;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.DocumentsContract;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;
import org.json.JSONObject;

/** Registers a persisted document tree; legacy copy transactions remain recoverable. */
final class ResourceImporter {
    interface Listener { void onImportState(State state); }
    static final class State {
        final boolean running;
        final String message;
        final int permille;
        final String importedId;
        State(boolean running, String message, int permille) { this(running, message, permille, null); }
        State(boolean running, String message, int permille, String importedId) {
            this.running = running; this.message = message; this.permille = permille; this.importedId = importedId;
        }
    }
    private static final Handler MAIN = new Handler(Looper.getMainLooper());
    private static final ExecutorService WORKER = Executors.newSingleThreadExecutor();
    private static final String STAGING_PREFIX = ".entis-import-";
    private static final String STAGING_MARKER = ".entis-import-staging";
    private static final String MARKER_VALUE = "EntisGLS library import v2\n";
    private static final String JOURNAL = ".entis-import-transaction.json";
    private static final String RECEIPT = "import-receipt.json";
    private static WeakReference<Listener> listener = new WeakReference<>(null);
    private static State state = new State(false, "", 0);
    private static AtomicBoolean cancellation;

    static synchronized boolean isRunning() { return state.running; }
    static synchronized void attach(Listener target) { listener = new WeakReference<>(target); target.onImportState(state); }
    static synchronized void detach(Listener target) { if (listener.get() == target) listener.clear(); }
    static synchronized void cancel() { if (cancellation != null) cancellation.set(true); }

    private static void publish(State value) {
        synchronized (ResourceImporter.class) { state = value; }
        MAIN.post(() -> {
            Listener target;
            State latest;
            synchronized (ResourceImporter.class) { target = listener.get(); latest = state; }
            if (target != null) target.onImportState(latest);
        });
    }

    static void begin(Context context, Uri tree) {
        final AtomicBoolean canceled;
        synchronized (ResourceImporter.class) {
            if (state.running) return;
            canceled = new AtomicBoolean(false);
            cancellation = canceled;
            state = new State(true, "正在检查所选游戏目录…", -1);
        }
        publish(state);
        final Context app = context.getApplicationContext();
        WORKER.execute(() -> perform(app, tree, canceled));
    }

    private static final class Document {
        final Uri uri;
        final long size;
        final boolean directory;
        Document(Uri uri, long size, boolean directory) { this.uri = uri; this.size = size; this.directory = directory; }
    }

    private static Map<String, Document> discover(ContentResolver resolver, Uri tree, AtomicBoolean canceled) throws IOException {
        Map<String, Document> found = new LinkedHashMap<>();
        discoverDirectory(resolver, tree, DocumentsContract.getTreeDocumentId(tree), "", found, new HashSet<>(), canceled, 0);
        boolean candidate = false;
        for (Map.Entry<String, Document> entry : found.entrySet()) {
            if (!entry.getValue().directory && entry.getValue().size != 0 && !entry.getKey().contains("/")
                    && ResourceStore.configurationCandidate(entry.getKey())) candidate = true;
        }
        if (!candidate) throw new IOException("未找到启动配置候选。请选择直接包含游戏 EXE、cotopha.xml、entis-launcher.xml、.csx 或 .noa 的游戏目录。是否支持该游戏将在启动时检查。");
        return found;
    }

    private static void discoverDirectory(ContentResolver resolver, Uri tree, String documentId, String prefix,
            Map<String, Document> found, Set<String> visited, AtomicBoolean canceled, int depth) throws IOException {
        if (canceled.get()) throw new CancellationException();
        if (depth > 64 || !visited.add(documentId)) throw new IOException("所选目录层级过深或包含循环引用。");
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(tree, documentId);
        String[] columns = { DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_DISPLAY_NAME,
            DocumentsContract.Document.COLUMN_SIZE, DocumentsContract.Document.COLUMN_MIME_TYPE };
        Set<String> names = new HashSet<>();
        // Close provider cursors before recursion; some providers limit concurrent queries.
        Map<String, String> directories = new LinkedHashMap<>();
        try (Cursor cursor = resolver.query(children, columns, null, null, null)) {
            if (cursor == null) throw new IOException("无法读取所选文件夹，请重新选择。");
            while (cursor.moveToNext()) {
                if (canceled.get()) throw new CancellationException();
                String name = cursor.getString(1);
                if (name == null || name.isEmpty() || name.equals(".") || name.equals("..")
                        || name.contains("/") || name.contains("\\") || name.indexOf('\0') >= 0)
                    throw new IOException("来源包含无法安全访问的文件名。");
                if (!names.add(name.toLowerCase(Locale.ROOT))) throw new IOException("发现仅大小写不同的重复资源：" + prefix + name);
                String id = cursor.getString(0);
                if (id == null) throw new IOException("文件提供方未返回资源标识。");
                Uri document = DocumentsContract.buildDocumentUriUsingTree(tree, id);
                boolean directory = DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(3));
                String path = prefix + name;
                found.put(path, new Document(document, cursor.isNull(2) ? -1 : cursor.getLong(2), directory));
                if (found.size() > 200000) throw new IOException("所选目录文件过多，请选择单个游戏目录。");
                if (directory) directories.put(path, id);
            }
        }
        for (Map.Entry<String, String> directory : directories.entrySet())
            discoverDirectory(resolver, tree, directory.getValue(), directory.getKey() + "/", found, visited, canceled, depth + 1);
    }

    private static void perform(Context context, Uri tree, AtomicBoolean canceled) {
        State completion;
        try {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);
            File root = ResourceStore.storageRoot(context);
            recover(root);
            DocumentTreeAccess access = new DocumentTreeAccess(context, tree);
            Map<String, Document> documents = discover(context.getContentResolver(), tree, canceled);
            // Probe random access without copying or hashing entire game archives.
            for (Map.Entry<String, Document> item : documents.entrySet()) {
                if (canceled.get()) throw new CancellationException();
                if (!item.getValue().directory && ResourceStore.configurationCandidate(item.getKey())) {
                    int fd = access.open(item.getKey(), "r");
                    try (android.os.ParcelFileDescriptor descriptor = android.os.ParcelFileDescriptor.adoptFd(fd)) {}
                }
            }
            if (canceled.get()) throw new CancellationException();
            ResourceStore.Game game = ResourceStore.registerTree(context, tree, access.displayName());
            completion = new State(false, "已添加「" + game.name + "」。游戏将直接读写所选文件夹，无需复制资源。", 1000, game.id);
        } catch (CancellationException error) {
            completion = new State(false, "添加已取消。", 0);
        } catch (Exception error) {
            String message = error.getMessage();
            completion = new State(false, "添加未完成：" + (message == null ? error.getClass().getSimpleName() : message), 0);
        } finally {
            synchronized (ResourceImporter.class) { cancellation = null; }
        }
        publish(completion);
    }

    static synchronized void recoverIfNeeded(Context context) throws IOException {
        if (!state.running) recover(ResourceStore.storageRoot(context));
    }

    private static void recover(File root) throws IOException {
        recoverLegacy(root);
        File journal = new File(root, JOURNAL);
        if (!journal.isFile()) return;
        try {
            JSONObject transaction = new JSONObject(readSmallText(journal));
            String id = transaction.getString("id");
            String stagingName = transaction.getString("staging");
            if (transaction.getInt("schema") != 2 || !ResourceStore.validId(id) || ResourceStore.LEGACY_ID.equals(id)
                    || !stagingName.equals(STAGING_PREFIX + id)) throw new IOException("游戏库恢复记录无效。");
            File library = new File(root, "library");
            ResourceStore.privateDirectory(library);
            File target = new File(library, id);
            File staging = new File(root, stagingName);
            if (!target.exists()) {
                if (!new File(staging, RECEIPT).isFile() || !new File(staging, ResourceStore.ENTRY).isFile()
                        || !new File(staging, "game").isDirectory() || !staging.renameTo(target))
                    throw new IOException("无法恢复已复制的游戏，请保留导入目录后重试。");
            }
            if (!new File(target, ResourceStore.ENTRY).isFile() || !new File(target, RECEIPT).isFile())
                throw new IOException("游戏库恢复目标不完整，请保留资源目录。");
            new File(target, STAGING_MARKER).delete();
            if (!journal.delete()) throw new IOException("无法完成游戏库恢复记录清理。");
        } catch (IOException error) { throw error; }
        catch (Exception error) { throw new IOException("无法读取游戏库恢复记录。", error); }
    }

    /** Complete the previous single-game import transaction before exposing old data. */
    private static void recoverLegacy(File root) throws IOException {
        File journal = new File(root, ".resource-import-transaction.json");
        if (!journal.isFile()) return;
        try {
            JSONObject transaction = new JSONObject(readSmallText(journal));
            String backupName = transaction.optString("backup");
            String stagingName = transaction.optString("staging");
            if (!stagingName.startsWith(".game-import-") || !safeName(stagingName)
                    || (!backupName.isEmpty() && (!backupName.startsWith("game-backup-") || !safeName(backupName))))
                throw new IOException("旧版资源恢复记录无效。");
            File game = new File(root, "game");
            if (!game.exists() && !backupName.isEmpty()) {
                File backup = new File(root, backupName);
                if (!backup.isDirectory() || !backup.renameTo(game)) throw new IOException("无法恢复旧资源，请保留 " + backupName);
            } else if (!game.exists()) {
                File ready = new File(root, stagingName);
                if (!new File(ready, ".noa-import.json").isFile() || !ready.renameTo(game))
                    throw new IOException("无法恢复旧版已复制的资源，请保留资源目录。");
            }
            new File(game, ".studysteady-import-staging").delete();
            if (!journal.delete()) throw new IOException("无法清理旧版资源恢复记录。");
        } catch (IOException error) { throw error; }
        catch (Exception error) { throw new IOException("无法读取旧版资源恢复记录。", error); }
    }

    private static boolean safeName(String name) { return !name.contains("/") && !name.contains("\\") && name.indexOf('\0') < 0; }

    private static void removeCreatedTree(File directory) {
        File[] children = directory.listFiles();
        if (children != null) for (File child : children) {
            // Never follow a link out of a staging directory, even if storage was altered externally.
            try {
                if (child.isDirectory() && child.getCanonicalFile().equals(new File(directory.getCanonicalFile(), child.getName())))
                    removeCreatedTree(child);
                else child.delete();
            } catch (IOException ignored) { /* Leave unrecognized content for manual recovery. */ }
        }
        directory.delete();
    }

    private static void writeText(File path, String value) throws IOException {
        try (FileOutputStream output = new FileOutputStream(path)) {
            output.write(value.getBytes(StandardCharsets.UTF_8));
            output.getFD().sync();
        }
    }

    static void writeAtomically(File path, String value) throws IOException {
        File temporary = new File(path.getParentFile(), path.getName() + ".tmp");
        writeText(temporary, value);
        if (!temporary.renameTo(path)) throw new IOException("无法保存游戏库事务记录。");
    }

    static String readSmallText(File path) throws IOException {
        if (path.length() > 65536) throw new IOException("导入记录过大。");
        try (FileInputStream input = new FileInputStream(path)) {
            byte[] bytes = new byte[(int) path.length()];
            int position = 0;
            while (position < bytes.length) {
                int read = input.read(bytes, position, bytes.length - position);
                if (read < 0) throw new IOException("导入记录不完整。");
                position += read;
            }
            return new String(bytes, StandardCharsets.UTF_8);
        }
    }

    private ResourceImporter() {}
}
