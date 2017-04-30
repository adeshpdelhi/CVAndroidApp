package com.example.adesh.myapplication;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import static android.R.attr.src;

public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2{

    private static String TAG="MainActivity";
    Mat mRGB;
    public native static String Wrapper(long addMat);
    JavaCameraView javaCameraView;
    BaseLoaderCallback mLoaderCallBack =new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch(status){
                case BaseLoaderCallback.SUCCESS:{
                    javaCameraView.enableView();
                    break;
                }
                default:{
                    super.onManagerConnected(status);
                    break;
                }

            }

        }
    };

    static{
        if(OpenCVLoader.initDebug())
        {
            Log.d(TAG,"Loaded Success");
            System.loadLibrary("Project_Library");
        }
        else {
            Log.d(TAG,"Failed");
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        javaCameraView=(JavaCameraView)findViewById(R.id.java_camera_view);
        javaCameraView.setVisibility(SurfaceView.VISIBLE);
        javaCameraView.setCvCameraViewListener(this);
//        javaCameraView.setRotation(30.0f);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(javaCameraView!=null)
            javaCameraView.disableView();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(OpenCVLoader.initDebug())
        {
            Log.d(TAG,"Loaded Success");
            mLoaderCallBack.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
        else {
            Log.d(TAG,"Failed");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_9,this,mLoaderCallBack);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(javaCameraView!=null)
            javaCameraView.disableView();
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        mRGB=new Mat(width, height, CvType.CV_8UC4);

    }

    @Override
    public void onCameraViewStopped() {
        mRGB.release();
    }
    static int counter = 0;
    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        mRGB=inputFrame.rgba();
        counter++;
//        if(counter%5 !=0)
//            return mRGB;
        String output = Wrapper(mRGB.getNativeObjAddr()).toString();
//        Toast.makeText(getApplicationContext(),output,Toast.LENGTH_SHORT).show();
        Log.d("Main Activity", mRGB.rows()+", "+mRGB.cols());
        Log.d("Main Activity", output);
        return mRGB;
    }
}