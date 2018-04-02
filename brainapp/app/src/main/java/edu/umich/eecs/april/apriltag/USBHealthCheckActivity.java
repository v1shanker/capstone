package edu.umich.eecs.april.apriltag;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.List;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

public class USBHealthCheckActivity extends AppCompatActivity {

    private TextView mDevicesText;
    private UsbSerialDriver mDriver;
    private UsbDeviceConnection mConnection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_usbhealth_check);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        mDevicesText = findViewById(R.id.devicestext);

        initSerial();

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mConnection == null || mDriver == null) {
                    Snackbar.make(view, "No active connection!", Snackbar.LENGTH_LONG)
                            .setAction("Action", null).show();
                    return;
                }

                UsbSerialPort port = mDriver.getPorts().get(0);
                try {
                    port.open(mConnection);
                    port.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

                    port.write("AT?".getBytes(), 2000);

                    byte buffer[] = new byte[16];
                    int numBytesRead = port.read(buffer, 2000);

                    String read = new String(buffer, Charset.forName("US-ASCII"));
                    if (numBytesRead >= 3 && read.startsWith("AT!")) {
                        Snackbar.make(view, "Successful health check!", Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    } else {
                        Snackbar.make(view, Arrays.toString(buffer), Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    }

                    port.close();
                } catch (IOException e) {
                    Snackbar.make(view, "Error opening connection!", Snackbar.LENGTH_LONG)
                            .setAction("Action", null).show();
                }
            }
        });
    }

    private void initSerial() {
        UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> drivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);

        if (drivers.isEmpty()) { return; }

        StringBuilder sb = new StringBuilder();
        for (UsbSerialDriver driver : drivers) {
            sb.append(driver.getDevice().getDeviceName());
            sb.append("\n");
            mDriver = driver;
        }
        mDevicesText.setText(sb.toString());

        if (mDriver != null) {
            mConnection = manager.openDevice(mDriver.getDevice());
        }
    }

}
