package edu.umich.eecs.april.apriltag;

import android.content.Context;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.nfc.Tag;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Queue;
import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * Created by DSimo on 4/23/2018.
 */

class BodyConnection {
    private static final String TAG = "BodyConnection";
    private static BodyConnection sInstance = null;

    private boolean mConnected = false;
    private UsbSerialPort mPort = null;

    private final static String MOTOR_START_CMD = "MSTART";
    private final static String MOTOR_STOP_CMD = "MSTOP";
    private final static byte END_CHAR = (byte)0xA;

    private Queue<Byte> inputQueue = new ArrayBlockingQueue<>(256);
    private int linesReady = 0;

    public static BodyConnection getInstance() {
        if (sInstance == null) {
            sInstance = new BodyConnection();
        }
        return sInstance;
    }

    public boolean isConnected() {
        return mConnected;
    }

    public void connect(Context context) {
        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber()
                .findAllDrivers(manager);
        if (availableDrivers.isEmpty()) { return; }

        // Open a connection to the first available driver
        UsbSerialDriver driver = availableDrivers.get(0);
        UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
        if (connection == null) { return; }

        mPort = driver.getPorts().get(0);
        try {
            mPort.open(connection);
            mPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            mConnected = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void disconnect() {
        mConnected = false;
        try {
            mPort.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void send(String s) {
        try {
            mPort.write(s.getBytes(), 100);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String getLine() {
        byte c;
        byte[] buf = new byte[256];
        int readBytes = 0;
        while ((c = inputQueue.remove()) != END_CHAR) {
            buf[readBytes] = c;
            readBytes++;
        };
        linesReady--;

        return new String(buf, 0, readBytes, StandardCharsets.US_ASCII);
    }

    private void handleLine(String s) {
        if (s.startsWith("L") && !s.startsWith("LFAIL")) {
            Scanner scanner = new Scanner(s.substring(1));
            int l50 = scanner.nextInt();
            int l30 = scanner.nextInt();
            int l10 = scanner.nextInt();
            int r10 = scanner.nextInt();
            int r30 = scanner.nextInt();
            int r50 = scanner.nextInt();

        }
    }

    public String handleInput() {
        try {
            byte[] buf = new byte[256];
            int bytesRead = mPort.read(buf, 100);
            for (int i = 0; i < bytesRead; i++) {
                byte c = buf[i];
                inputQueue.add(c);
                if (c == END_CHAR) {
                    linesReady++;
                }
            }
            for (int i = 0; i < linesReady; i++) {
                handleLine(getLine());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }
}
