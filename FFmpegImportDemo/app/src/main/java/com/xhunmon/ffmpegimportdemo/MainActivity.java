package com.xhunmon.ffmpegimportdemo;

import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.xhunmon.ffmpegimportdemo.core.Player;


public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView txtShow = findViewById(R.id.txtShow);
        txtShow.setText(Player.getFFVersion());
    }
}