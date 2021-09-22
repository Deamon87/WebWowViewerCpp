package com.deamon.wow.viewer;

import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;

import java.util.concurrent.LinkedBlockingQueue;

public class MainActivity extends NativeActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    void showSoftInput() {
        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.showSoftInput(this.getWindow().getDecorView(), 0);
    }

    void hideSoftInput() {
        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(this.getWindow().getDecorView().getWindowToken(), 0);
    }

    // Queue for the Unicode characters to be polled from native code (via pollUnicodeChar())
    private LinkedBlockingQueue<Integer> unicodeCharacterQueue = new LinkedBlockingQueue<Integer>();

    // We assume dispatchKeyEvent() of the NativeActivity is actually called for every
    // KeyEvent and not consumed by any View before it reaches here
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            unicodeCharacterQueue.offer(event.getUnicodeChar(event.getMetaState()));
        }
        return super.dispatchKeyEvent(event);
    }

    int pollUnicodeChar() {
        Integer a = unicodeCharacterQueue.poll();
        return (a != null) ? a : 0;
    }
}
