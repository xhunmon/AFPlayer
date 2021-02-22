package cn.qincji.afplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

import cn.qincji.afplayer.core.AFPlayer;
import cn.qincji.afplayer.core.callback.OnAFCallback;


public class MainActivity extends AppCompatActivity implements OnAFCallback {

    private static final int REQUEST_PERMISSIONS = 0x01;
    private String srcPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/Kobe.flv";
//    private String srcPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/lol.mp4";
    private TextView txtShow;
    private SurfaceView surfaceView;
    private AFPlayer afPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        txtShow = findViewById(R.id.txtShow);
        surfaceView = findViewById(R.id.surfaceView);
        txtShow.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                return false;
            }
        });
        if (requestPermission()) {
//            init();
        }
    }

    private void init() {
        afPlayer = new AFPlayer();
        afPlayer.setStatusCallback(this);
        File file = new File(srcPath);
        if (!file.exists()) {
            Log.e("main", "文件不存在：" + srcPath);
            return;
        }
        Log.d("main", "文件存在：" + srcPath);
        afPlayer.setDataSource(file,surfaceView.getHolder().getSurface());
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
//                init();
            } else {
                Toast.makeText(MainActivity.this, "申请权限失败！", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onInitialized() {

    }

    @Override
    public void onPrepared() {
        Log.d("afplayer_app","底层给上层的onPrepared回调");
        txtShow.setText("height："+afPlayer.getHeight()+" | width："+afPlayer.getWidth()+" | duration："+afPlayer.getDuration());
    }

    @Override
    public void onStarted() {

    }

    @Override
    public void onPaused() {

    }

    @Override
    public void onStopped() {

    }

    @Override
    public void onSeek(long curDuration) {

    }

    @Override
    public void onPlaying(long curDuration) {

    }

    @Override
    public void onError(int code, String msg) {
        Log.d("afplayer_app","底层给上层的onError回调: "+code+" "+msg);
    }

    @Override
    public void onReleased() {

    }


    public void start(View view) {
        afPlayer.start();
    }

    public void pause(View view) {
    }

    public void stop(View view) {
    }

    public void init(View view) {
        init();
    }
}