package edu.umich.eecs.april.apriltag;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import java.nio.ByteBuffer;

public final class BitmapUtils {
    private BitmapUtils(){}

    public static byte[] convertBitmapToByteArray(Bitmap bitmap){
        ByteBuffer byteBuffer = ByteBuffer.allocate(bitmap.getByteCount());
        bitmap.copyPixelsToBuffer(byteBuffer);
        byteBuffer.rewind();
        return byteBuffer.array();
    }

    public static Bitmap convertByteArrayToBitmap(byte[] src){
        return BitmapFactory.decodeByteArray(src, 0, src.length);
    }

    public static Bitmap getMirroredImage(Bitmap src) {
        Matrix mtx = new Matrix();
        mtx.preScale(-1.0f, 1.0f);
        Bitmap mirroredImage = Bitmap.createBitmap(src,0,0,src.getWidth(), src.getHeight(), mtx, true);
        return mirroredImage;
    }
}
