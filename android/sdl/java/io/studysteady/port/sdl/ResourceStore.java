package io.studysteady.port.sdl;

import android.content.Context;
import android.system.ErrnoException;
import android.system.Os;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import org.json.JSONObject;

/** App-owned game library. Only native code decides whether a game is supported. */
final class ResourceStore {
    static final String LEGACY_ID = "legacy";
    static final String ENTRY = "entry.json";

    static final class Game {
        final String id;
        final String name;
        final File directory;
        final boolean legacy;
        Game(String id, String name, File directory, boolean legacy) {
            this.id = id; this.name = name; this.directory = directory; this.legacy = legacy;
        }
        @Override public String toString() { return name; }
    }

    static File storageRoot(Context context) throws IOException {
        File root = context.getExternalFilesDir(null);
        if (root == null) throw new IOException("应用资源存储暂不可用，请稍后重试。");
        return root;
    }

    static File localRoot(Context context) throws IOException {
        File root = context.getFilesDir();
        privateDirectory(root);
        return root;
    }

    static void privateDirectory(File directory) throws IOException {
        if (!directory.isDirectory() && !directory.mkdirs()) throw new IOException("无法创建应用目录。");
        try { Os.chmod(directory.getAbsolutePath(), 0700); }
        catch (ErrnoException error) {
            // Emulated external storage may expose fixed modes. Actual app IO,
            // rather than chmod support, determines whether the copy is usable.
            if (!directory.canRead() || !directory.canWrite()) throw new IOException("应用目录不可读写。", error);
        }
    }

    static boolean validId(String id) {
        return id != null && (LEGACY_ID.equals(id) || id.matches("[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}"));
    }

    static List<Game> games(Context context) throws IOException {
        ResourceImporter.recoverIfNeeded(context);
        File root = storageRoot(context);
        ArrayList<Game> games = new ArrayList<>();
        File previous = new File(root, "game");
        if (previous.isDirectory()) games.add(new Game(LEGACY_ID, "已有游戏（旧版资源）", previous, true));
        File library = new File(root, "library");
        if (!library.exists()) return games;
        File[] directories = library.listFiles();
        if (directories == null) throw new IOException("无法读取游戏库。");
        for (File directory : directories) {
            if (!directory.isDirectory() || !validId(directory.getName()) || LEGACY_ID.equals(directory.getName())) continue;
            File game = new File(directory, "game");
            File metadata = new File(directory, ENTRY);
            if (!game.isDirectory() || !metadata.isFile()) continue;
            try {
                JSONObject entry = new JSONObject(ResourceImporter.readSmallText(metadata));
                if (!directory.getName().equals(entry.getString("id"))) continue;
                String name = entry.optString("name", directory.getName());
                games.add(new Game(directory.getName(), name, game, false));
            } catch (Exception error) {
                android.util.Log.w("EntisGLS", "Cannot read game entry " + directory.getName(), error);
            }
        }
        Collections.sort(games, (left, right) -> left.name.compareToIgnoreCase(right.name));
        return games;
    }

    static Game game(Context context, String id) throws IOException {
        if (!validId(id)) throw new IOException("请选择要启动的游戏。");
        for (Game game : games(context)) if (game.id.equals(id)) return game;
        throw new IOException("所选游戏已不可用，请重新选择或导入。");
    }

    static boolean configurationCandidate(String name) {
        String lower = name.toLowerCase(Locale.ROOT);
        return lower.equals("cotopha.xml") || lower.equals("entis-launcher.xml") || lower.endsWith(".exe") || lower.endsWith(".csx") || lower.endsWith(".noa");
    }

    /** Check permissions as the app, without claiming to recognize an engine. */
    static String readiness(Context context, Game game) {
        if (game == null) return "请选择一个游戏，或导入游戏文件夹。";
        try {
            int files = checkReadable(game.directory, 0);
            if (files == 0) return "游戏文件夹为空，请重新导入完整游戏目录。";
            if (!game.legacy) {
                boolean candidate = false;
                File[] entries = game.directory.listFiles();
                if (entries == null) throw new IOException("无法读取游戏目录。");
                for (File file : entries)
                    if (file.isFile() && file.length() > 0 && configurationCandidate(file.getName())) candidate = true;
                if (!candidate) return "未找到启动配置候选。请选择直接包含游戏 EXE、cotopha.xml、entis-launcher.xml、.csx 或 .noa 的目录。";
            }
            localRoot(context);
            return null;
        } catch (IOException error) { return error.getMessage(); }
    }

    private static int checkReadable(File directory, int depth) throws IOException {
        if (depth > 64) throw new IOException("游戏目录层级过深。");
        File[] files = directory.listFiles();
        if (files == null) throw new IOException("无法读取资源目录：" + directory.getName());
        int count = 0;
        for (File file : files) {
            if (Thread.currentThread().isInterrupted()) throw new IOException("资源检查已取消。");
            if (!file.getCanonicalFile().equals(new File(directory.getCanonicalFile(), file.getName())))
                throw new IOException("资源包含非应用导入的链接，请重新导入。");
            if (file.isDirectory()) { count += checkReadable(file, depth + 1); continue; }
            if (!file.isFile()) throw new IOException("资源不是普通文件：" + file.getName());
            try (FileInputStream input = new FileInputStream(file)) {
                if (input.read() < 0 && file.length() > 0) throw new IOException("资源无法读取：" + file.getName());
            } catch (IOException error) { throw new IOException("游戏无法读取 " + file.getName() + "，请重新导入资源。", error); }
            ++count;
        }
        return count;
    }

    private ResourceStore() {}
}
