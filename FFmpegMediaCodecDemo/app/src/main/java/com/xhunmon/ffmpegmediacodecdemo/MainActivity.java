package com.xhunmon.ffmpegmediacodecdemo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.xhunmon.ffmpegmediacodecdemo.core.Player;

import java.io.File;


public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_PERMISSIONS = 0x01;
    private String srcPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/Kobe.flv";
    private String dstPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/Kobe-384x216.yuv";
    private TextView txtShow;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        txtShow = findViewById(R.id.txtShow);
        if (requestPermission()) {
            init();
        }
    }

    private void init() {
        Player player = new Player();
        txtShow.setText(player.getFFVersion());
        File file = new File(srcPath);
        if (!file.exists()) {
            Log.e("main", "文件不存在：" + srcPath);
            return;
        }
        Log.d("main", "文件存在：" + srcPath);
        player.decode_file(srcPath, dstPath);
    }

    private boolean requestPermission() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        requestPermissions(new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE}, REQUEST_PERMISSIONS);
        return false;
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                init();
            } else {
                Toast.makeText(MainActivity.this, "申请权限失败！", Toast.LENGTH_SHORT).show();
            }
        }
    }
}