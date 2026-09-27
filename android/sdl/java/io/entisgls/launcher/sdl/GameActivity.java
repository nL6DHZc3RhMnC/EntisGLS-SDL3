package io.entisgls.launcher.sdl;

import android.media.AudioManager;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.widget.Toast;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import org.libsdl.app.SDLActivity;

/** SDL owns rendering, input, audio and lifecycle; this class only supplies paths. */
public final class GameActivity extends SDLActivity {
    private DocumentTreeAccess documentTreeAccess;
    /** Called by native code after SDL has obtained getArguments(). */
    public DocumentTreeAccess getDocumentTreeAccess() { return documentTreeAccess; }

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    @Override protected String[] getLibraries() {
        return new String[] { "c++_shared", "SDL3", "main" };
    }

    @Override protected void main() {
        try { super.main(); }
        catch (IllegalStateException error) {
            // A provider can disappear between the launcher's readiness check
            // and this SDL thread. Let SDL return to the library gracefully.
            android.util.Log.e("EntisGLS", "Cannot start selected game", error);
            runOnUiThread(() -> Toast.makeText(getApplicationContext(),
                "无法启动游戏：" + error.getMessage(), Toast.LENGTH_LONG).show());
        }
    }

    @Override protected String[] getArguments() {
        try {
            File local = ResourceStore.localRoot(this);
            ResourceStore.Game game = ResourceStore.game(this, getIntent().getStringExtra("game_id"));
            documentTreeAccess = game.tree == null ? null : new DocumentTreeAccess(this, game.tree);
            ArrayList<String> arguments = new ArrayList<>(Arrays.asList(
                "--game-dir", game.directory.getAbsolutePath(),
                "--storage-dir", ResourceStore.storageRoot(this).getAbsolutePath(),
                "--local-dir", local.getAbsolutePath()));
            String psbKey = PsbKeySettings.read(this, game.id);
            if (psbKey != null) {
                arguments.add("--psb-key");
                arguments.add(psbKey);
            }
            if ((getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0) {
                String probe = getIntent().getStringExtra("probe");
                if (Arrays.asList("self-test", "window-probe", "emote-probe", "image-export-probe", "opening-probe").contains(probe))
                    arguments.add("--" + probe);
                int exitAfter = getIntent().getIntExtra("exit_after", 0);
                if (exitAfter > 0 && exitAfter <= 300) {
                    arguments.add("--exit-after"); arguments.add(Integer.toString(exitAfter));
                }
                if (getIntent().hasExtra("capture_after")) {
                    int captureAfter = getIntent().getIntExtra("capture_after", -1);
                    if (captureAfter >= 0 && captureAfter <= 300) {
                        arguments.add("--capture-frame"); arguments.add(new File(local, "sdl-frame.png").getAbsolutePath());
                        arguments.add("--capture-after"); arguments.add(Integer.toString(captureAfter));
                    }
                }
            }
            return arguments.toArray(new String[0]);
        } catch (IOException error) {
            // The launcher checks these paths before starting SDL. A storage
            // disappearance must be reported rather than using another root.
            throw new IllegalStateException(error.getMessage(), error);
        }
    }
}
