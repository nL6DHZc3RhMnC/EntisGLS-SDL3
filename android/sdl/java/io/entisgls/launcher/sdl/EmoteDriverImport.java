package io.entisgls.launcher.sdl;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.system.ErrnoException;
import android.system.Os;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.util.Locale;

/** Copies a user-selected DLL as data only; native code recognizes its contents. */
final class EmoteDriverImport {
    private static final long LIMIT = 256L * 1024 * 1024;

    static void copy(Context context, String gameId, Uri source) throws IOException {
        ResourceStore.Game game = ResourceStore.game(context, gameId);
        File root = game.directory.getCanonicalFile();
        if (!root.equals(new File(game.directory.getParentFile().getCanonicalFile(), game.directory.getName())))
            throw new IOException("游戏资源目录包含链接，请重新导入。");
        ContentResolver resolver = context.getContentResolver();
        long expected = -1;
        try (Cursor cursor = resolver.query(source, new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE }, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int nameColumn = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                int sizeColumn = cursor.getColumnIndex(OpenableColumns.SIZE);
                String name = nameColumn >= 0 ? cursor.getString(nameColumn) : null;
                if (name == null || !name.toLowerCase(Locale.ROOT).endsWith(".dll"))
                    throw new IOException("请选择原游戏的 E-mote DLL 文件。");
                if (sizeColumn >= 0 && !cursor.isNull(sizeColumn)) expected = cursor.getLong(sizeColumn);
            }
        } catch (SecurityException error) { throw new IOException("无法读取所选文件，请重新选择。", error); }
        if (expected > LIMIT) throw new IOException("驱动文件超过 256 MiB，无法导入。");
        File temporary = File.createTempFile(".emote-driver-", ".tmp", root);
        try {
            long copied = 0;
            try (InputStream input = resolver.openInputStream(source); FileOutputStream output = new FileOutputStream(temporary)) {
                if (input == null) throw new IOException("无法打开所选文件。");
                byte[] buffer = new byte[65536];
                for (;;) {
                    if (Thread.currentThread().isInterrupted()) throw new IOException("驱动导入已取消，请重试。");
                    int count = input.read(buffer);
                    if (count < 0) break;
                    copied += count;
                    if (copied > LIMIT) throw new IOException("驱动文件超过 256 MiB，无法导入。");
                    output.write(buffer, 0, count);
                }
                output.getFD().sync();
            }
            if (expected >= 0 && copied != expected) throw new IOException("文件长度与来源报告不符，请重试。");
            validatePe(temporary);
            File destination = new File(root, "emotedriver.dll");
            if (!destination.getCanonicalFile().equals(destination)) throw new IOException("目标驱动是符号链接，无法覆盖。");
            if (destination.exists() && !destination.isFile()) throw new IOException("目标 emotedriver.dll 不是普通文件。");
            if (destination.length() > LIMIT) throw new IOException("已有驱动超过 256 MiB，无法安全备份。");
            // Keep the replaced app-owned copy. Complete the backup first, so
            // even interruption before the atomic rename leaves the old DLL.
            if (destination.exists()) {
                File backup = File.createTempFile(".emotedriver-backup-", ".bin", root);
                try (FileInputStream input = new FileInputStream(destination); FileOutputStream output = new FileOutputStream(backup)) {
                    byte[] buffer = new byte[65536];
                    int count;
                    while ((count = input.read(buffer)) >= 0) {
                        if (Thread.currentThread().isInterrupted()) throw new IOException("驱动导入已取消，请重试。");
                        output.write(buffer, 0, count);
                    }
                    output.getFD().sync();
                }
            }
            if (Thread.currentThread().isInterrupted()) throw new IOException("驱动导入已取消，请重试。");
            rename(temporary, destination);
        } catch (SecurityException error) {
            throw new IOException("无法读取所选文件，请重新选择。", error);
        } finally {
            if (temporary.exists() && !temporary.delete())
                android.util.Log.w("EntisGLS", "Could not remove temporary driver copy", null);
        }
    }

    private static void rename(File source, File target) throws IOException {
        try { Os.rename(source.getAbsolutePath(), target.getAbsolutePath()); }
        catch (ErrnoException error) { throw new IOException("无法保存驱动文件，请重试。", error); }
    }

    private static void validatePe(File file) throws IOException {
        try (RandomAccessFile input = new RandomAccessFile(file, "r")) {
            if (input.length() < 64 || input.readUnsignedByte() != 'M' || input.readUnsignedByte() != 'Z')
                throw new IOException("所选文件不是有效的 Windows DLL（缺少 MZ 头）。");
            input.seek(60);
            long offset = Integer.toUnsignedLong(Integer.reverseBytes(input.readInt()));
            if (offset < 64 || offset > input.length() - 24) throw new IOException("所选 DLL 的 PE 头无效。");
            input.seek(offset);
            if (input.readInt() != 0x50450000) throw new IOException("所选 DLL 的 PE 签名无效。");
            input.seek(offset + 22);
            int characteristics = Short.toUnsignedInt(Short.reverseBytes(input.readShort()));
            if ((characteristics & 0x2000) == 0) throw new IOException("所选 PE 文件不是 DLL。");
        }
    }

    private EmoteDriverImport() {}
}
