package com.xhunmon.openslesdemo;

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

import com.xhunmon.openslesdemo.core.Player;

import java.io.File;


public class MainActivity extends AppCompatActivity {

    private static final int PERMISSION_REQUEST = 0x01;
    private TextView txtShow;
    private String path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/NocturneNo2inEflat_44.1k_s16le.pcm";
//    private String path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/little-endian_2_44100_16.pcm";

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
        File file = new File(path);
        if(!file.exists()){
            Log.e("main","文件不存在："+path);
            return;
        }
        Log.d("main","文件存在："+path);
        new Player().play_audio(path,44100,2,16);
    }

    private boolean requestPermission() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        requestPermissions(new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE}, PERMISSION_REQUEST);
        return false;
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                init();
            } else {
                Toast.makeText(MainActivity.this, "申请相机失败！", Toast.LENGTH_SHORT).show();
            }
        }
    }
}