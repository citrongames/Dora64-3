package com.n64recomp.dora64three;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.content.res.ColorStateList;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.libsdl.app.SDLActivity;

public final class Dora64ThreeActivity extends SDLActivity {
    private static final int PICK_ROM = 64;
    private static final long ROM_SIZE = 16L * 1024L * 1024L;
    private static final String TAG = "Dora64-3";
    private LinearLayout shaderCompilationOverlay;

    // Called from the native render thread; this view is drawn by Android,
    // so it remains visible and animated while Vulkan compiles its pipelines.
    public void setShaderCompilationVisible(boolean visible) {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;
            if (shaderCompilationOverlay == null) {
                if (!visible) return;
                float density = getResources().getDisplayMetrics().density;
                LinearLayout row = new LinearLayout(this);
                row.setOrientation(LinearLayout.HORIZONTAL);
                row.setGravity(Gravity.CENTER_VERTICAL);
                row.setPadding((int)(24 * density), (int)(18 * density),
                    (int)(24 * density), (int)(18 * density));
                GradientDrawable background = new GradientDrawable();
                background.setColor(0xE6222222);
                background.setCornerRadius(12 * density);
                row.setBackground(background);
                row.setElevation(8 * density);
                ProgressBar spinner = new ProgressBar(this);
                spinner.setIndeterminateTintList(ColorStateList.valueOf(Color.WHITE));
                row.addView(spinner, new LinearLayout.LayoutParams((int)(28 * density), (int)(28 * density)));
                TextView label = new TextView(this);
                label.setText("Compiling shaders…");
                label.setTextColor(Color.WHITE);
                label.setTextSize(18);
                label.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
                LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                textParams.setMarginStart((int)(16 * density));
                row.addView(label, textParams);
                FrameLayout content = findViewById(android.R.id.content);
                content.addView(row, new FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.CENTER));
                shaderCompilationOverlay = row;
            }
            shaderCompilationOverlay.setVisibility(visible ? View.VISIBLE : View.GONE);
        });
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "Dora64Three" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        new File(getFilesDir(), ".assets-ready").delete();
        new File(getFilesDir(), ".rom-selection-canceled").delete();
        super.onCreate(savedInstanceState);

        try {
            File assetsRoot = new File(getFilesDir(), "assets");
            copyAssetTree("dora64three/assets", assetsRoot);
            File ready = new File(getFilesDir(), ".assets-ready");
            if (!ready.exists() && !ready.createNewFile()) {
                throw new IOException("Cannot mark assets ready");
            }
        } catch (IOException error) {
            Log.e(TAG, "Could not prepare game assets", error);
            markSelectionCanceled();
            new AlertDialog.Builder(this)
                .setTitle("Dora64-3")
                .setMessage("Could not prepare game assets: " + error.getMessage())
                .setPositiveButton("Close", (dialog, which) -> finish())
                .setCancelable(false)
                .show();
            return;
        }

        File rom = new File(getFilesDir(), "dora3.n64.jp.z64");
        if (!rom.isFile()) {
            getWindow().getDecorView().post(this::selectRom);
        }
    }

    private void copyAssetTree(String source, File destination) throws IOException {
        String[] children = getAssets().list(source);
        if (children == null) {
            throw new IOException("Cannot list " + source);
        }
        if (children.length == 0) {
            File parent = destination.getParentFile();
            if (parent == null || (!parent.isDirectory() && !parent.mkdirs())) {
                throw new IOException("Cannot create " + parent);
            }
            try (InputStream input = getAssets().open(source);
                 OutputStream output = new FileOutputStream(destination)) {
                byte[] buffer = new byte[64 * 1024];
                int count;
                while ((count = input.read(buffer)) != -1) {
                    output.write(buffer, 0, count);
                }
            }
            return;
        }
        if (!destination.isDirectory() && !destination.mkdirs()) {
            throw new IOException("Cannot create " + destination);
        }
        for (String child : children) {
            copyAssetTree(source + "/" + child, new File(destination, child));
        }
    }

    private void selectRom() {
        File canceled = new File(getFilesDir(), ".rom-selection-canceled");
        if (canceled.exists() && !canceled.delete()) {
            Log.w(TAG, "Could not clear ROM selection marker");
        }
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        startActivityForResult(intent, PICK_ROM);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != PICK_ROM) {
            return;
        }
        if (resultCode != RESULT_OK || data == null || data.getData() == null) {
            markSelectionCanceled();
            finish();
            return;
        }

        File rom = new File(getFilesDir(), "dora3.n64.jp.z64");
        MessageDigest digest;
        try { digest = MessageDigest.getInstance("SHA-256"); }
        catch (NoSuchAlgorithmException error) { showRomError("SHA-256 is unavailable"); return; }
        File temporary = new File(getFilesDir(), "dora3.n64.jp.z64.part");
        try (InputStream input = getContentResolver().openInputStream(data.getData());
             OutputStream output = new FileOutputStream(temporary)) {
            if (input == null) {
                throw new IOException("Cannot open the selected file");
            }
            byte[] buffer = new byte[64 * 1024];
            long total = 0;
            int count;
            while ((count = input.read(buffer)) != -1) {
                digest.update(buffer, 0, count);
                total += count;
                if (total > ROM_SIZE) {
                    throw new IOException("The selected file is larger than 16 MiB");
                }
                output.write(buffer, 0, count);
            }
            if (total != ROM_SIZE) {
                throw new IOException("The selected file must be exactly 16 MiB");
            }
        } catch (IOException error) {
            temporary.delete();
            showRomError(error.getMessage());
            return;
        }
        StringBuilder hash = new StringBuilder();
        for (byte value : digest.digest()) hash.append(String.format(java.util.Locale.ROOT, "%02x", value & 255));
        if (!hash.toString().equals("410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602")) {
            temporary.delete();
            showRomError("Select the original Japanese Doraemon 3 ROM (.z64).");
            return;
        }
        if (!temporary.renameTo(rom)) {
            temporary.delete();
            showRomError("Cannot save the selected ROM");
        }
    }

    private void showRomError(String message) {
        new AlertDialog.Builder(this)
            .setTitle("Dora64-3")
            .setMessage(message)
            .setPositiveButton("Choose again", (dialog, which) -> selectRom())
            .setNegativeButton("Close", (dialog, which) -> {
                markSelectionCanceled();
                finish();
            })
            .setCancelable(false)
            .show();
    }

    private void markSelectionCanceled() {
        try {
            new File(getFilesDir(), ".rom-selection-canceled").createNewFile();
        } catch (IOException error) {
            Log.e(TAG, "Could not mark ROM selection canceled", error);
        }
    }
}
