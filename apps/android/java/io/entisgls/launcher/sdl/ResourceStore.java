package io.entisgls.launcher.sdl;

import android.content.Context;
import android.net.Uri;
import android.system.ErrnoException;
import android.system.Os;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.UUID;
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
        final Uri tree;
        Game(String id, String name, File directory, boolean legacy) {
            this(id, name, directory, legacy, null);
        }
        Game(String id, String name, File directory, boolean legacy, Uri tree) {
            this.id = id; this.name = name; this.directory = directory; this.legacy = legacy; this.tree = tree;
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
        readLibrary(new File(root, "library"), games);
        readLibrary(new File(localRoot(context), "library"), games);
        Collections.sort(games, (left, right) -> left.name.compareToIgnoreCase(right.name));
        return games;
    }

    private static void readLibrary(File library, List<Game> games) throws IOException {
        if (!library.exists()) return;
        File[] directories = library.listFiles();
        if (directories == null) throw new IOException("无法读取游戏库。");
        for (File directory : directories) {
            if (!directory.isDirectory() || !validId(directory.getName()) || LEGACY_ID.equals(directory.getName())) continue;
            File metadata = new File(directory, ENTRY);
            if (!metadata.isFile()) continue;
            try {
                JSONObject entry = new JSONObject(ResourceImporter.readSmallText(metadata));
                if (!directory.getName().equals(entry.getString("id"))) continue;
                String name = entry.optString("name", directory.getName());
                String source = entry.optString("source_tree", "");
                if (!source.isEmpty()) {
                    Uri tree = Uri.parse(source);
                    games.add(new Game(directory.getName(), name, new File("/__entis_saf__/" + directory.getName()), false, tree));
                } else {
                    File game = new File(directory, "game");
                    if (game.isDirectory()) games.add(new Game(directory.getName(), name, game, false));
                }
            } catch (Exception error) {
                android.util.Log.w("EntisGLS", "Cannot read game entry " + directory.getName(), error);
            }
        }
    }

    static Game registerTree(Context context, Uri tree, String name) throws IOException {
        for (Game game : games(context)) if (tree.equals(game.tree)) return game;
        String id = UUID.randomUUID().toString();
        File directory = new File(new File(localRoot(context), "library"), id);
        privateDirectory(directory);
        try {
            ResourceImporter.writeAtomically(new File(directory, ENTRY), new JSONObject().put("schema", 2).put("id", id)
                .put("name", name).put("source_tree", tree.toString()).put("added_at_ms", System.currentTimeMillis()).toString(2));
        } catch (IOException error) { throw error; }
        catch (Exception error) { throw new IOException("无法保存所选游戏文件夹。", error); }
        return new Game(id, name, new File("/__entis_saf__/" + id), false, tree);
    }

    static Game game(Context context, String id) throws IOException {
        if (!validId(id)) throw new IOException("请选择要启动的游戏。");
        for (Game game : games(context)) if (game.id.equals(id)) return game;
        throw new IOException("所选游戏已不可用，请重新选择文件夹。");
    }

    static boolean configurationCandidate(String name) {
        String lower = name.toLowerCase(Locale.ROOT);
        return lower.equals("cotopha.xml") || lower.equals("entis-launcher.xml") || lower.endsWith(".exe") || lower.endsWith(".csx") || lower.endsWith(".noa");
    }

    /** Check permissions as the app, without claiming to recognize an engine. */
    static String readiness(Context context, Game game) {
        if (game == null) return "请选择一个游戏，或添加游戏文件夹。";
        try {
            if (game.tree != null) {
                DocumentTreeAccess access = new DocumentTreeAccess(context, game.tree);
                boolean candidate = false;
                for (String name : access.list("")) {
                    if (!configurationCandidate(name)) continue;
                    long[] info = access.stat(name);
                    if (info[0] == 1 && info[1] != 0) {
                        int fd = access.open(name, "r");
                        try (android.os.ParcelFileDescriptor descriptor = android.os.ParcelFileDescriptor.adoptFd(fd)) {}
                        candidate = true;
                    }
                }
                if (!candidate) return "未找到启动配置候选，请重新选择直接包含游戏 EXE、XML、.csx 或 .noa 的目录。";
                return null;
            }
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
        } catch (IOException | RuntimeException error) {
            return error.getMessage() == null ? "无法访问游戏文件夹，请重新选择并确认读写授权。" : error.getMessage();
        }
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
