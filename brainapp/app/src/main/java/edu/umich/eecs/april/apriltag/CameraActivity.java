package edu.umich.eecs.april.apriltag;

import android.Manifest;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.WindowManager;

import android.os.Build;
import android.widget.FrameLayout.LayoutParams;
import android.widget.RelativeLayout;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 */
public class CameraActivity extends AppCompatActivity {

    private static final String TAG = "AprilTag";
    /**
     * 'camera' is the object that references the hardware device
     * installed on your Android phone.
     */
    private Camera camera;

    /**
     * Phone can have multiple cameras, so 'cameraID' is a
     * useful variable to store which one of the camera is active.
     * It starts with value -1
     */
    private int cameraID;

    /**
     * 'camPreview' is the object that prints the data
     * coming from the active camera on the GUI, that is... frames!
     * It's an instance of the 'CameraPreview' class, more information
     * in {@link CameraPreview}
     */
    private CameraPreview camPreview;

    private int has_camera_permissions = 0;
    private static final int MY_PERMISSIONS_REQUEST_CAMERA = 77;

    private void verifyPreferences() {
        SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        int nthreads = Integer.parseInt(sharedPreferences.getString("nthreads_value", "0"));
        if (nthreads <= 0) {
            int nproc = Runtime.getRuntime().availableProcessors();
            if (nproc <= 0) {
                nproc = 1;
            }
            Log.i(TAG, "available processors: " + nproc);
            PreferenceManager.getDefaultSharedPreferences(this).edit().putString("nthreads_value", Integer.toString(nproc)).apply();
        }
    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Ensure we have permission to use the camera (Permission Requesting for Android 6.0/SDK 23 and higher)
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            // Assume user knows enough about the app to know why we need the camera, just ask for permission
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.CAMERA},
                    MY_PERMISSIONS_REQUEST_CAMERA);
        } else {
            this.has_camera_permissions = 1;
        }

        setContentView(R.layout.main);

        // Make the screen stay awake
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    /** Release the camera when application focus is lost */
    protected void onPause() {
        super.onPause();
        releaseCameraInstance();
    }

    /** (Re-)initialize the camera */
    protected void onResume() {
        super.onResume();
        if (has_camera_permissions == 1 && setCameraInstance() == true) {
            this.camPreview = new CameraPreview(this, this.camera, this.cameraID);
            // if the preview is set, we add it to the contents of our activity.
            RelativeLayout preview = (RelativeLayout) findViewById(R.id.preview_layout);
            if (this.camPreview != null) {
                preview.addView(this.camPreview);
            }

            // also we set some layout properties
            RelativeLayout.LayoutParams previewLayout = (RelativeLayout.LayoutParams) camPreview.getLayoutParams();
            previewLayout.width = LayoutParams.MATCH_PARENT;
            previewLayout.height = LayoutParams.MATCH_PARENT;
            this.camPreview.setLayoutParams(previewLayout);
        }
        else {
            Log.e(TAG, "onResume(): can't reconnect the camera");
            this.finish();
        }

        Camera.Size cameraSize = this.camera.getParameters().getPreviewSize();
        // launch the drive service
        Intent serviceLaunchIntent = new Intent(this, DriveService.class);
//        serviceLaunchIntent.putExtra(DriveService.WIDTH, cameraSize.width);
//        serviceLaunchIntent.putExtra(DriveService.HEIGHT, cameraSize.height);
        startService(serviceLaunchIntent);
    }


    @Override
    protected void onStop() {
        super.onStop();

        Intent intent = new Intent(this, DriveService.class);
        stopService(intent);
    }

    /**
     * [IMPORTANT!] The most important method of this Activity: it asks for an instance
     * of the hardware camera(s) and save it to the private field {@link #camera}.
     * @return TRUE if camera is set, FALSE if something bad happens
     */
    private boolean setCameraInstance() {
        if (this.camera != null) {
            // do the job only if the camera is not already set
            Log.i(TAG, "setCameraInstance(): camera is already set, nothing to do");
            return true;
        }

        // warning here! starting from API 9, we can retrieve one from the multiple
        // hardware cameras (ex. front/back)
        if (Build.VERSION.SDK_INT >= 9) {

            if (this.cameraID < 0) {
                // at this point, it's the first time we request for a camera
                Camera.CameraInfo camInfo = new Camera.CameraInfo();
                for (int i = 0; i < Camera.getNumberOfCameras(); i++) {
                    Camera.getCameraInfo(i, camInfo);

                    if (camInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                        // in this example we'll request specifically the back camera
                        try {
                            this.camera = Camera.open(i);
                            this.cameraID = i; // assign to cameraID this camera's ID (O RLY?)
                            return true;
                        }
                        catch (Exception e){
                            // something bad happened! this camera could be locked by other apps
                            Log.e(TAG, "setCameraInstance(): trying to open camera #" + i + " but it's locked", e);
                            return false;
                        }
                    }
                }
            }
            else {
                // at this point, a previous camera was set, we try to re-instantiate it
                try {
                    this.camera = Camera.open(this.cameraID);
                }
                catch (Exception e){
                    Log.e(TAG, "setCameraInstance(): trying to re-open camera #" + this.cameraID + " but it's locked", e);
                    return false;
                }
            }
        }

        // we could reach this point in two cases:
        // - the API is lower than 9
        // - previous code block failed
        // hence, we try the classic method, that doesn't ask for a particular camera
        if (this.camera == null) {
            try {
                this.camera = Camera.open();
                this.cameraID = 0;
            }
            catch (Exception e) {
                // this is REALLY bad, the camera is definitely locked by the system.
                Log.e(TAG,
                        "setCameraInstance(): trying to open default camera but it's locked. "
                                + "The camera is not available for this app at the moment.", e
                );
                return false;
            }
        }

        // here, the open() went good and the camera is available
        Log.i(TAG, "setCameraInstance(): successfully set camera #" + this.cameraID);
        return true;
    }

    /**
     * [IMPORTANT!] Another very important method: it releases all the resources and the locks
     * we created while using the camera. It MUST be called everytime the app exits, crashes,
     * is paused or whatever. The order of the called methods are the following: <br />
     *
     * 1) stop any preview coming to the GUI, if running <br />
     * 2) call {@link Camera#release()} <br />
     * 3) set our camera object to null and invalidate its ID
     */
    private void releaseCameraInstance() {
        if (this.camera != null) {
            try {
                this.camera.stopPreview();
            }
            catch (Exception e) {
                Log.i(TAG, "releaseCameraInstance(): tried to stop a non-existent preview, this is not an error");
            }

            this.camera.setPreviewCallback(null);
            this.camera.release();
            this.camera = null;
            this.cameraID = -1;
            Log.i(TAG, "releaseCameraInstance(): camera has been released.");
        }
    }

    public Camera getCamera() {
        return this.camera;
    }

    public int getCameraID() {
        return this.cameraID;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case MY_PERMISSIONS_REQUEST_CAMERA: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Log.i(TAG, "App GRANTED camera permissions");

                    RelativeLayout preview = (RelativeLayout) findViewById(R.id.preview_layout);
                    if (this.camPreview != null) {
                        preview.addView(this.camPreview);
                    }
                    // Set flag
                    this.has_camera_permissions = 1;

                    // Restart the TagViewer
                    SurfaceView overlayView = new SurfaceView(this);

                    // Restart the camera
                    onPause();
                    onResume();
                } else {
                    Log.i(TAG, "App DENIED camera permissions");
                    this.has_camera_permissions = 0;
                }
                return;
            }
        }
    }
}
