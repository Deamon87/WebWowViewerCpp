package com.deamon.wow.viewer;

import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.concurrent.LinkedBlockingQueue;

public class MainActivity extends NativeActivity {

    static {
        System.loadLibrary("AWebWoWViewerCpp");
    }

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

    public void downloadFile(String url, long userDataForRequest) {
        try {
            URL urlObj = new URL(url);
            URLConnection urlConnection = urlObj.openConnection();
            InputStream in = urlConnection.getInputStream();

            byte[] data = readAllBytes(in);
//            downloadFailed(userDataForRequest);
            downloadSucceeded(userDataForRequest, data);
        } catch (Exception e) {
            System.out.println(e.getMessage());
            downloadFailed(userDataForRequest);
        }

    }

    public static byte[] readAllBytes(InputStream inputStream) throws IOException {
        final int bufLen = 4 * 0x400; // 4KB
        byte[] buf = new byte[bufLen];
        int readLen;
        IOException exception = null;

        try {
            try (ByteArrayOutputStream outputStream = new ByteArrayOutputStream()) {
                while ((readLen = inputStream.read(buf, 0, bufLen)) != -1)
                    outputStream.write(buf, 0, readLen);

                return outputStream.toByteArray();
            }
        } catch (IOException e) {
            exception = e;
            throw e;
        } finally {
            if (exception == null) inputStream.close();
            else try {
                inputStream.close();
            } catch (IOException e) {
                exception.addSuppressed(e);
            }
        }
    }

    public native void downloadSucceeded(long userDataPtr, byte[] data );
    public native void downloadFailed(long userDataPtr);
}
