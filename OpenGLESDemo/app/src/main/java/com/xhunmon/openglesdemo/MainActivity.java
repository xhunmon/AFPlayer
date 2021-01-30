package com.xhunmon.openglesdemo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.xhunmon.openglesdemo.core.Player;

import java.io.File;


public class MainActivity extends AppCompatActivity {

    private static final int PERMISSION_REQUEST_CAMERA = 0x01;
    private TextView txtShow;
    private SurfaceView surfaceView;
    private String path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/a-frame-384x216.rgb24";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        txtShow = findViewById(R.id.txtShow);
        surfaceView = findViewById(R.id.surfaceView);
        if (requestPermission()) {
            init();
        }
    }

    private void init() {
        File file = new File(path);
        if (!file.exists()) {
            Log.e("main", "文件不存在：" + path);
        }
    }

    private boolean requestPermission() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }
        if (checkSelfPermission(Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO,
                Manifest.permission.CAMERA,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE}, PERMISSION_REQUEST_CAMERA);
        return false;
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CAMERA) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                init();
            } else {
                Toast.makeText(MainActivity.this, "申请相机失败！", Toast.LENGTH_SHORT).show();
            }
        }
    }

    public void preview(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                new Player().play_video(surfaceView.getHolder().getSurface(), path);
            }
        }).start();
    }

    public void push(View view) {
    }

    public void stop(View view) {
    }

    public void switchCamera(View view) {
    }

    public void write(View view) {
    }
}