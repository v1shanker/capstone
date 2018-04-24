package edu.umich.eecs.april.apriltag;

import android.content.Context;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.IOException;
import java.util.List;

/**
 * Created by DSimo on 4/23/2018.
 */

class BodyConnection {
    private static BodyConnection sInstance = null;

    private boolean mConnected = false;
    private UsbSerialPort mPort = null;

    private final static String MOTOR_START_CMD = "MSTART";
    private final static String MOTOR_STOP_CMD = "MSTOP";

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


}
