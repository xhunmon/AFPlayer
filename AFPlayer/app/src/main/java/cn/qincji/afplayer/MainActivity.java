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
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.bytedance.raphael.Raphael;

import cn.qincji.afplayer.core.AFPlayer;
import cn.qincji.afplayer.core.DisplayUtil;
import cn.qincji.afplayer.core.callback.OnAFCallback;


public class MainActivity extends AppCompatActivity implements OnAFCallback {

    private static final int REQUEST_PERMISSIONS = 0x01;
    private String srcPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/input.mp4";
    //    private String srcPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/lol.mp4";
    private TextView txtShow;
    private SurfaceView surfaceView;
    private AFPlayer afPlayer;
    private SeekBar timeSeekBar;
    private long totalTime;
    private TextView txtCurTime;
    private TextView txtTotalTime;
    private DisplayUtil displayUtil;
    private int position = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        displayUtil = new DisplayUtil();
        txtShow = findViewById(R.id.txtShow);
        surfaceView = findViewById(R.id.surfaceView);
        txtCurTime = findViewById(R.id.txtCurTime);
        txtTotalTime = findViewById(R.id.txtTotalTime);
        timeSeekBar = findViewById(R.id.timeSeekBar);
        timeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                position = (int) (totalTime * progress / 100);
                txtCurTime.setText(displayUtil.duration2Time(progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
//                seekTo(position);
            }
        });
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
        afPlayer.setDataSource(srcPath, surfaceView.getHolder().getSurface());
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
        Log.d("afplayer_app", "底层给上层的onPrepared回调");
        txtShow.post(new Runnable() {
            @Override
            public void run() {
                txtShow.setText("height：" + afPlayer.getHeight() + " | width：" + afPlayer.getWidth() + " | duration：" + afPlayer.getDuration());
            }
        });
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
    public void onPlaying(long curDuration, long totalDuration) {
        timeSeekBar.post(new Runnable() {
            @Override
            public void run() {
                totalTime = totalDuration;
                timeSeekBar.setProgress((int) (curDuration * 100 / totalTime));
                txtCurTime.setText(DisplayUtil.secdsToDateFormat((int) curDuration));
                txtTotalTime.setText(DisplayUtil.secdsToDateFormat((int) totalDuration));
            }
        });
    }

    private void playCurrentTime(int currentTime, int totalTime) {

    }

    @Override
    public void onError(int code, String msg) {
        Log.d("afplayer_app", "底层给上层的onError回调: " + code + " " + msg);
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

    public void memoryStart(View view) {
        // 监控整个进程
        Raphael.start(
                Raphael.MAP64_MODE | Raphael.ALLOC_MODE | 0x0F0000 | 1024,
                "/storage/emulated/0/raphael", // need sdcard permission
                null
        );
        Raphael.print();
    }

    public void memoryEnd(View view) {
        Raphael.stop();
    }
}