package com.lxfly2000.rapidball;

import android.os.Process;
import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    @Override
    public void onDestroy(){
        super.onDestroy();
        Process.killProcess(Process.myPid());
    }
}
