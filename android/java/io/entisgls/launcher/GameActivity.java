package io.entisgls.launcher;

import android.os.Bundle;
import com.entis.android.entisgls4.EntisGLActivity;
import com.entis.android.entisgls4.EntisGLS;
import java.io.File;
import android.util.Log;

public final class GameActivity extends EntisGLActivity {
    @Override protected void onCreate(Bundle state) {
        new File(getExternalFilesDir(null), "game").mkdirs();
        // The SDK's Unix mkdir default omits directory search permission.
        // Prepare the app-owned save root before its environment opens files.
        File saves = new File(getFilesDir(), "savedata");
        if (!saves.isDirectory() && !saves.mkdirs())
            throw new IllegalStateException("Cannot create save directory");
        try {
            android.system.Os.chmod(saves.getAbsolutePath(), 0700);
        } catch (android.system.ErrnoException error) {
            throw new IllegalStateException("Cannot prepare save directory", error);
        }
        super.onCreate(state);
        setVolumeControlStream(android.media.AudioManager.STREAM_MUSIC);
        if (EntisGLS.isLoadedNativeLibrary())
            nativeConfigureAudio((android.media.AudioManager)getSystemService(AUDIO_SERVICE));
        // Development probe only; parsing success does not mean rendered playback.
        final String psb = new File(getExternalFilesDir(null), "game/haz_a.psb").getAbsolutePath();
        if (EntisGLS.isLoadedNativeLibrary()) {
            new Thread(new Runnable() {
                @Override public void run() {
                    Log.i("StudySteady", nativeCheckPsb(psb));
                }
            }, "PSB-check").start();
        }
    }
    private static native String nativeCheckPsb(String path);
    private static native void nativeConfigureAudio(android.media.AudioManager manager);
}
