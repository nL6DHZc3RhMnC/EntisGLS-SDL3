package io.studysteady.port.sdl;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputFilter;
import android.text.InputType;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

/** A library of user-selected games; native code validates and launches their configuration. */
public final class LauncherActivity extends Activity implements ResourceImporter.Listener {
    private static final int SELECT_RESOURCES = 20;
    private static final int SELECT_EMOTE_DRIVER = 21;
    private TextView status;
    private ProgressBar progress;
    private Button start;
    private Button select;
    private Button cancel;
    private Button psbSettings;
    private Button importDriver;
    private Spinner selector;
    private List<ResourceStore.Game> games = new ArrayList<>();
    private ResourceStore.Game selected;
    private String selectedId;
    private String handledImportId;
    private boolean autoStart;
    private boolean refreshing;
    private boolean resumed;
    private boolean launchRequested;
    private boolean driverImporting;
    private String driverTargetId;
    private int readinessGeneration;
    private final ExecutorService readinessWorker = Executors.newSingleThreadExecutor();
    private Future<?> readinessTask;
    private SharedPreferences preferences;

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        preferences = getSharedPreferences("sdl_resources", MODE_PRIVATE);
        selectedId = getIntent().getStringExtra("game_id");
        if (selectedId == null) selectedId = preferences.getString("selected_game", null);
        if (state != null) handledImportId = state.getString("handled_import");
        if (state != null) driverTargetId = state.getString("driver_target");
        autoStart = getIntent().getBooleanExtra("start_game", false);
        int padding = (int) (24 * getResources().getDisplayMetrics().density);
        LinearLayout body = new LinearLayout(this);
        body.setOrientation(LinearLayout.VERTICAL);
        body.setPadding(padding, padding, padding, padding);
        TextView title = new TextView(this);
        title.setText("EntisGLS Launcher");
        title.setTextSize(25);
        body.addView(title);
        TextView description = new TextView(this);
        description.setText("选择已导入的游戏，或导入单个游戏的完整目录。目录中的配置、资源包与字体会一并复制；EXE 仅用于读取引擎配置。游戏是否兼容将在启动时检查。导入时请保持此页面打开。");
        description.setPadding(0, padding / 2, 0, padding / 2);
        body.addView(description);
        selector = new Spinner(this);
        selector.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (refreshing || position < 0 || position >= games.size()) return;
                selected = games.get(position);
                selectedId = selected.id;
                preferences.edit().putString("selected_game", selectedId).apply();
                launchRequested = false;
                checkReadiness("");
            }
            @Override public void onNothingSelected(AdapterView<?> parent) { selected = null; }
        });
        body.addView(selector);
        status = new TextView(this);
        status.setTextIsSelectable(true);
        body.addView(status);
        progress = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        progress.setMax(1000);
        body.addView(progress);
        start = new Button(this);
        start.setText("启动所选游戏");
        start.setOnClickListener(view -> launchGame());
        body.addView(start);
        psbSettings = new Button(this);
        psbSettings.setText("设置 PSB 解码参数");
        psbSettings.setOnClickListener(view -> editPsbKey());
        body.addView(psbSettings);
        importDriver = new Button(this);
        importDriver.setText("补充 E-mote 驱动文件");
        importDriver.setOnClickListener(view -> selectEmoteDriver());
        body.addView(importDriver);
        select = new Button(this);
        select.setText("导入另一个游戏文件夹");
        select.setOnClickListener(view -> selectResources());
        body.addView(select);
        cancel = new Button(this);
        cancel.setText("取消导入");
        cancel.setOnClickListener(view -> ResourceImporter.cancel());
        body.addView(cancel);
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.addView(body);
        setContentView(scroll);
    }

    @Override protected void onSaveInstanceState(Bundle state) {
        state.putString("handled_import", handledImportId);
        state.putString("driver_target", driverTargetId);
        super.onSaveInstanceState(state);
    }

    @Override protected void onResume() {
        super.onResume();
        resumed = true;
        ResourceImporter.attach(this);
    }

    @Override protected void onPause() {
        resumed = false;
        cancelReadiness();
        ResourceImporter.detach(this);
        super.onPause();
    }

    @Override protected void onDestroy() {
        readinessWorker.shutdownNow();
        super.onDestroy();
    }

    private void selectResources() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, SELECT_RESOURCES);
    }

    private void editPsbKey() {
        if (selected == null || ResourceImporter.isRunning() || driverImporting) return;
        final ResourceStore.Game target = selected;
        final EditText input = new EditText(this);
        input.setSingleLine(true);
        input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
        input.setFilters(new InputFilter[] { new InputFilter.LengthFilter(64) });
        input.setHint("十进制或 0x 十六进制；留空恢复自动");
        try { input.setText(PsbKeySettings.read(this, target.id)); }
        catch (IOException error) { input.setError(error.getMessage()); }
        final AlertDialog dialog = new AlertDialog.Builder(this)
            .setTitle(target.name + "：PSB 解码参数")
            .setMessage("手动参数优先于游戏 XML 配置。留空时先使用 XML 参数，否则从游戏 DLL 自动发现。设置保存在启动器中，不修改游戏资源。")
            .setView(input).setPositiveButton("保存", null).setNegativeButton("取消", null)
            .setNeutralButton("恢复自动", null).create();
        dialog.setOnShowListener(ignored -> {
            dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(view -> savePsbKey(dialog, input, target, input.getText().toString()));
            dialog.getButton(AlertDialog.BUTTON_NEUTRAL).setOnClickListener(view -> savePsbKey(dialog, input, target, ""));
        });
        dialog.show();
    }

    private void savePsbKey(AlertDialog dialog, EditText input, ResourceStore.Game target, String text) {
        try {
            PsbKeySettings.write(this, target.id, text);
            dialog.dismiss();
            checkReadiness(PsbKeySettings.normalize(text) == null ? "已恢复自动参数来源。" : "已保存该游戏的手动 PSB 参数。参数会在加载动画时校验。");
        } catch (IOException error) { input.setError(error.getMessage()); }
    }

    private void selectEmoteDriver() {
        if (selected == null || ResourceImporter.isRunning() || driverImporting) return;
        driverTargetId = selected.id;
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        startActivityForResult(intent, SELECT_EMOTE_DRIVER);
    }

    private void importEmoteDriver(Uri source) {
        final String targetId = driverTargetId;
        driverTargetId = null;
        if (targetId == null || ResourceImporter.isRunning() || driverImporting) return;
        driverImporting = true;
        cancelReadiness();
        start.setEnabled(false);
        selector.setEnabled(false);
        select.setEnabled(false);
        psbSettings.setEnabled(false);
        importDriver.setEnabled(false);
        progress.setVisibility(View.VISIBLE);
        progress.setIndeterminate(true);
        status.setText("正在复制并检查 E-mote 驱动…");
        readinessWorker.submit(() -> {
            String message;
            try {
                EmoteDriverImport.copy(getApplicationContext(), targetId, source);
                message = "驱动已补充到所选游戏。启动时会读取并校验参数；不会执行 DLL。";
            } catch (IOException | RuntimeException error) { message = "驱动导入失败：" + error.getMessage(); }
            final String result = message;
            runOnUiThread(() -> {
                if (isDestroyed()) return;
                driverImporting = false;
                selector.setEnabled(true);
                select.setEnabled(true);
                psbSettings.setEnabled(selected != null);
                importDriver.setEnabled(selected != null);
                progress.setVisibility(View.GONE);
                checkReadiness(result);
            });
        });
    }

    @Override protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request == SELECT_EMOTE_DRIVER) {
            if (result == RESULT_OK && data != null && data.getData() != null) importEmoteDriver(data.getData());
            else driverTargetId = null;
            return;
        }
        if (request != SELECT_RESOURCES || result != RESULT_OK || data == null || data.getData() == null) return;
        Uri tree = data.getData();
        String previous = preferences.getString("source_tree", null);
        int readGrant = data.getFlags() & Intent.FLAG_GRANT_READ_URI_PERMISSION;
        try {
            getContentResolver().takePersistableUriPermission(tree, readGrant);
            preferences.edit().putString("source_tree", tree.toString()).apply();
            if (previous != null && !previous.equals(tree.toString())) {
                try { getContentResolver().releasePersistableUriPermission(Uri.parse(previous), Intent.FLAG_GRANT_READ_URI_PERMISSION); }
                catch (SecurityException ignored) { /* A provider may have revoked its previous grant. */ }
            }
        } catch (SecurityException | IllegalArgumentException ignored) {
            preferences.edit().remove("source_tree").apply();
        }
        ResourceImporter.begin(getApplicationContext(), tree);
    }

    private void refreshGames() throws IOException {
        games = ResourceStore.games(this);
        refreshing = true;
        try {
            ArrayAdapter<ResourceStore.Game> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, games);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            selector.setAdapter(adapter);
            int position = -1;
            for (int index = 0; index < games.size(); ++index)
                if (games.get(index).id.equals(selectedId)) position = index;
            if (position < 0 && !games.isEmpty()) position = 0;
            selected = position >= 0 ? games.get(position) : null;
            if (selected != null) {
                selectedId = selected.id;
                preferences.edit().putString("selected_game", selectedId).apply();
                selector.setSelection(position);
            }
            selector.setVisibility(games.isEmpty() ? View.GONE : View.VISIBLE);
            psbSettings.setEnabled(selected != null && !driverImporting);
            importDriver.setEnabled(selected != null && !driverImporting);
        } finally { refreshing = false; }
    }

    private void cancelReadiness() {
        ++readinessGeneration;
        if (readinessTask != null) readinessTask.cancel(true);
        readinessTask = null;
    }

    private void checkReadiness(String message) {
        cancelReadiness();
        if (!resumed || ResourceImporter.isRunning() || driverImporting) return;
        start.setEnabled(false);
        final ResourceStore.Game target = selected;
        final int generation = readinessGeneration;
        status.setText(message.isEmpty() ? "正在检查资源…" : message);
        // A generic game may contain many loose files. Keep app-level permission
        // checks off the UI thread and discard stale selection/lifecycle results.
        readinessTask = readinessWorker.submit(() -> {
            String problem = ResourceStore.readiness(getApplicationContext(), target);
            runOnUiThread(() -> {
                if (!resumed || generation != readinessGeneration || selected != target || ResourceImporter.isRunning() || driverImporting) return;
                start.setEnabled(problem == null);
                String result = problem == null ? "资源可读取。启动时将识别配置并检查兼容性。" : problem;
                status.setText(message.isEmpty() ? result : message + "\n" + result);
                if (launchRequested || autoStart) {
                    launchRequested = false;
                    autoStart = false;
                    if (problem == null && target != null) launchSelectedGame(target);
                }
            });
        });
    }

    private void launchGame() {
        if (ResourceImporter.isRunning() || driverImporting || selected == null) return;
        launchRequested = true;
        checkReadiness("");
    }

    private void launchSelectedGame(ResourceStore.Game target) {
        try { PsbKeySettings.read(this, target.id); }
        catch (IOException error) { status.setText(error.getMessage()); return; }
        Intent game = new Intent(this, GameActivity.class);
        game.putExtra("game_id", target.id);
        Intent request = getIntent();
        if (request.hasExtra("probe")) game.putExtra("probe", request.getStringExtra("probe"));
        if (request.hasExtra("exit_after")) game.putExtra("exit_after", request.getIntExtra("exit_after", 0));
        if (request.hasExtra("capture_after")) game.putExtra("capture_after", request.getIntExtra("capture_after", 0));
        startActivity(game);
    }

    @Override public void onImportState(ResourceImporter.State state) {
        if (driverImporting) return;
        boolean busy = state.running;
        cancelReadiness();
        if (busy) getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        else getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        selector.setEnabled(!busy);
        select.setEnabled(!busy);
        psbSettings.setEnabled(!busy && selected != null);
        importDriver.setEnabled(!busy && selected != null);
        cancel.setVisibility(busy ? View.VISIBLE : View.GONE);
        progress.setVisibility(busy ? View.VISIBLE : View.GONE);
        progress.setIndeterminate(state.permille < 0);
        if (state.permille >= 0) progress.setProgress(state.permille);
        start.setEnabled(false);
        if (busy) { status.setText(state.message); return; }
        if (state.importedId != null && !state.importedId.equals(handledImportId)) {
            selectedId = state.importedId;
            handledImportId = state.importedId;
        }
        try {
            refreshGames();
            checkReadiness(state.message);
        } catch (IOException error) { status.setText(error.getMessage()); autoStart = false; }
    }
}
