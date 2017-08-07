package com.projects.alshell.vokaturi;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Build;
import android.util.Log;

import java.io.File;
import java.io.IOException;

/**
 * Created by Alshell7 @(Ashraf Khan Workstation)
 * 01:21 PM.
 * 05/Aug/2017
 */

public final class Vokaturi
{

    public static boolean DEBUG_LOGGING_ENABLED = true;
    public static final String TAG = "VokaturiLib";

    private String FileToAnalyze;
    private Context mContext;
    private WavAudioRecorder recorder;

    @SuppressLint("StaticFieldLeak")
    private static Vokaturi instance = null;

    private Vokaturi(Context context) {
        this.mContext = context;
        initialise();
    }

    private void initialise()
    {
        System.loadLibrary("vokaturi-lib");
    }

    public synchronized static Vokaturi getInstance(Context context) throws VokaturiException
    {
        if(instance == null){
            try {
                instance = new Vokaturi(context);
                logD("Vokaturi library initialised successfully");
            } catch (UnsatisfiedLinkError e) {
                logE("Vokaturi library initialising failed");
                throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "There was some problem initialising the library");
            }
        }
        return instance;
    }

    private void setFileToAnalyse(String fileName){
        logD("Setting file to analyze: " + fileName);
        this.FileToAnalyze = fileName;
    }

    /*public synchronized void startRecordingAudio(String fileName) throws VokaturiException
    {
        startRecordingInWav(fileName);
    }*/

    public synchronized void startRecordingAudio() throws VokaturiException
    {
        //passing null in suffix to create unique temporary files each time when recording
        logD("About to begin recording the audio to analyze");
        File outputFile = null;
        try {
            outputFile = File.createTempFile("vokaturi_recording", null, mContext.getCacheDir());
            setFileToAnalyse(outputFile.getAbsolutePath());
            startRecordingInWav();
        } catch (IOException e) {
            throw new VokaturiException(VokaturiException.VOKATURI_ERROR, e.getMessage());
        }

    }

    public File getRecordedAudio()
    {
        if (FileToAnalyze == null)
            throw new IllegalStateException("The API has not processed any file yet.");
        return new File(FileToAnalyze);
    }

    private void startRecordingInWav() throws VokaturiException
    {

        if (!shouldAskForPermissionsForGrants(mContext)) {
            logD("Permissions have been granted");

            recorder = WavAudioRecorder.getInstance(false);
            recorder.setOutputFile(FileToAnalyze);
            recorder.prepare();
            recorder.start();
            logD("WavAudioRecorder just started recording the audio");
        } else {
            logE("Some denied permissions found");
            throw new VokaturiException(VokaturiException.VOKATURI_DENIED_PERMISSIONS, "File Read Write/Recording Audio permissions not granted");
        }
    }

    private void stopRecordingWav(){
        if(recorder != null)
        {
            recorder.stop();
            recorder.reset();
            recorder.release();
            recorder = null;
            logD("Stopped the WavAudioRecorder");
        }
    }

    public EmotionProbabilities stopRecordingAndAnalyze() throws VokaturiException {
        //to analyse and send the data
        stopRecordingWav();

        return AnalyzeForEmotions(mContext, FileToAnalyze);
        //return new EmotionProbabilities();
    }

    private synchronized EmotionProbabilities AnalyzeForEmotions(Context context, String fileName) throws VokaturiException
    {
        if(shouldAskForPermissionsForGrants(context))
        {
            logE("Some denied permissions found");
            throw new VokaturiException(VokaturiException.VOKATURI_DENIED_PERMISSIONS, "File Read Write/Recording Audio permissions not granted");
        }
        logD("About to call native method #analyzeWav# to process the recorded file");

        return analyzeWav(fileName);
    }

    public static void AnalyzeForEmotionsAsync(Context context, String fileName, VokaturiResultCallback callback) throws VokaturiException
    {
        if(shouldAskForPermissionsForGrants(context))
        {
            throw new VokaturiException("File Read Write/Recording Audio permissions not granted");
        }
        new AsyncEmotionAnalyzerTask(callback).execute(fileName);
    }

    private static class AsyncEmotionAnalyzerTask extends AsyncTask<String, Void, Void>
    {
        private VokaturiResultCallback resultCallback;

        AsyncEmotionAnalyzerTask(VokaturiResultCallback callback)
        {
            this.resultCallback = callback;
        }

        @Override
        protected Void doInBackground(String... strings)
        {
            String fileToAnalyze = strings[0];
            try {
                EmotionProbabilities probabilities = analyzeWav(fileToAnalyze);
                resultCallback.onSuccess(probabilities);
            } catch (VokaturiException e) {
                resultCallback.onError(e);
            }

            return null;
        }
    }

    private static  boolean shouldAskForPermissionsForGrants(Context context)
    {
        if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            logD("Checking for required permissions");
            int resReadExternal = context.checkCallingOrSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
            int resWriteExternal = context.checkCallingOrSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            int resRecordAudio = context.checkCallingOrSelfPermission(Manifest.permission.RECORD_AUDIO);
            return  !(resReadExternal == PackageManager.PERMISSION_GRANTED
                    && resWriteExternal == PackageManager.PERMISSION_GRANTED
                    && resRecordAudio == PackageManager.PERMISSION_GRANTED
            );
        }
        return false;
    }


    public static void logD(String msg){
        if (DEBUG_LOGGING_ENABLED) {
            Log.d(TAG, msg);
        }
    }
    public static void logE(String msg){
        if (DEBUG_LOGGING_ENABLED) {
            Log.e(TAG, msg);
        }
    }

 /*   private static EmotionProbabilities analyzeWavNative(String fileToAnalyze) throws VokaturiException
    {
        try {
            analyzeWav(fileToAnalyze);
        } catch (PendingEx e) {
            e.printStackTrace();
        }
    }*/
    private static native EmotionProbabilities analyzeWav(String fileName) throws VokaturiException;

}
