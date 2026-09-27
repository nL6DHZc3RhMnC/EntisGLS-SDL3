package io.studysteady.port.sdl;

import android.content.Context;
import android.content.SharedPreferences;
import java.io.IOException;

/** A user override belongs to a library entry, never to its resource files. */
final class PsbKeySettings {
    private static final String PREFERENCES = "psb_key_overrides";

    static String normalize(String text) throws IOException {
        if (text == null) return null;
        if (text.length() > 64) throw invalid();
        String value = text.trim();
        if (value.isEmpty()) return null;
        boolean hex = value.startsWith("0x") || value.startsWith("0X");
        String digits = hex ? value.substring(2) : value;
        if (!digits.matches(hex ? "[0-9a-fA-F]+" : "[0-9]+")) throw invalid();
        try {
            long parsed = Long.parseLong(digits, hex ? 16 : 10);
            if (parsed > 0xffffffffL) throw invalid();
            return Long.toString(parsed);
        } catch (NumberFormatException error) { throw invalid(); }
    }

    private static IOException invalid() {
        return new IOException("请输入 0～4294967295 的十进制数或以 0x 开头的十六进制数；留空则恢复自动。");
    }

    static String read(Context context, String gameId) throws IOException {
        if (!ResourceStore.validId(gameId)) throw new IOException("请选择要设置的游戏。");
        Object value = context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE).getAll().get(gameId);
        if (value == null) return null;
        if (!(value instanceof String)) throw new IOException("该游戏的 PSB 参数设置损坏，请重新设置或恢复自动。");
        return normalize((String) value);
    }

    static void write(Context context, String gameId, String text) throws IOException {
        if (!ResourceStore.validId(gameId)) throw new IOException("请选择要设置的游戏。");
        String value = normalize(text);
        SharedPreferences.Editor editor = context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE).edit();
        if (value == null) editor.remove(gameId);
        else editor.putString(gameId, value);
        if (!editor.commit()) throw new IOException("无法保存 PSB 参数，请重试。");
    }

    private PsbKeySettings() {}
}
