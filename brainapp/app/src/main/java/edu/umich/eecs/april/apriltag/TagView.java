package edu.umich.eecs.april.apriltag;

import android.content.Context;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class TagView extends SurfaceView implements Camera.PreviewCallback {
    private static final String TAG = "AprilTag";
    private Camera mCamera;
    private Camera.Size mPreviewSize;
    private ByteBuffer mYuvBuffer;
    private ArrayList<ApriltagDetection> mDetections;
    private SystemState systemState;
    private SurfaceHolder surfaceHolder;

    public TagView(Context context, SurfaceHolder overlay) {
        super(context);
        overlay.setFormat(PixelFormat.TRANSPARENT);
        this.surfaceHolder = overlay;
    }

    public void setCamera(Camera camera)
    {
        Log.e(TAG, "1");
        if (camera == mCamera)
            return;

        // Stop the previous camera preview
        if (mCamera != null) {
            try {
                mCamera.stopPreview();
                //Log.i(TAG, "Camera stop");
            } catch (Exception e) { }
        }

        Log.e(TAG, "2");

        // Start the new mCamera preview
        if (camera != null) {
            Log.e(TAG, "3");
            setHighestCameraPreviewResolution(camera);

            // Ensure space for frame (12 bits per pixel)
            mPreviewSize = camera.getParameters().getPreviewSize();
            Log.i(TAG, "camera preview PreviewSize: " + mPreviewSize.width + "x" + mPreviewSize.height);
            int nbytes = mPreviewSize.width * mPreviewSize.height * 3 / 2; // XXX: What's the 3/2 scaling for?
            if (mYuvBuffer == null || mYuvBuffer.capacity() < nbytes) {
                // Allocate direct byte buffer so native code access won't require a copy
                Log.i(TAG, "Allocating buf of mPreviewSize " + nbytes);
                mYuvBuffer = ByteBuffer.allocateDirect(nbytes);
            }

            Log.e(TAG, "4");
            camera.addCallbackBuffer(mYuvBuffer.array());
            try {
                camera.setPreviewDisplay(surfaceHolder);
            } catch (IOException e) {
                throw new RuntimeException("FAILURE");
            }
            Log.e(TAG, "5");
            camera.setPreviewCallbackWithBuffer(this);
            camera.startPreview();
            //Log.i(TAG, "Camera start");
        }
        Log.e(TAG, "6");
        mCamera = camera;
    }

    private void setHighestCameraPreviewResolution(Camera camera)
    {
        Camera.Parameters parameters = camera.getParameters();
        List<Camera.Size> sizeList = camera.getParameters().getSupportedPreviewSizes();

        Camera.Size bestSize = sizeList.get(0);
        for (int i = 1; i < sizeList.size(); i++){
            if ((sizeList.get(i).width * sizeList.get(i).height) > (bestSize.width * bestSize.height)){
                bestSize = sizeList.get(i);
            }
        }

        parameters.setPreviewSize(bestSize.width, bestSize.height);
        Log.i(TAG, "Setting " + bestSize.width + " x " + bestSize.height);

        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        camera.setParameters(parameters);
    }

    static class ProcessingThread extends Thread {
        byte[] bytes;
        int width;
        int height;
        TagView parent;

        public void run() {
            parent.mDetections = ApriltagNative.apriltag_detect_yuv(bytes, width, height);
            Log.e(TAG, "C");
            parent.systemState.setDetectedTagList(parent.mDetections);
        }
    }

    public void onPreviewFrame(byte[] bytes, Camera camera) {
        Log.e(TAG, "A");
        // Check if mCamera has been released in another thread
        if (this.mCamera == null)
            return;
        Log.e(TAG, "B");
        // Spin up another thread so we don't block the UI thread
        ProcessingThread thread = new ProcessingThread();
        thread.bytes = bytes;
        thread.width = mPreviewSize.width;
        thread.height = mPreviewSize.height;
        thread.parent = this;
        thread.run();
    }

    public Camera.Size getmPreviewSize() {
        return mPreviewSize;
    }
}
