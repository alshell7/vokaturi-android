package com.projects.alshell.vokaturi;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.IOException;

/**
 * A class that provides interface between the native implementation of the Vokaturi emotion analysis engine
 * and offers basic functionality to make use of the library.
 */
public final class Vokaturi
{
    /**
     *A global flag to control whether to do the logging or not by the entire library.
     */
    public static boolean DEBUG_LOGGING_ENABLED = true;

    public static final String TAG = "VokaturiLib";

    //WAV file location in the cache directory of the application
    private String FileToAnalyze;

    private Context mContext;
    private WavAudioRecorder recorder;

    private RecordingAudioState recordingAudioState = RecordingAudioState.NOT_LISTENING;

    @SuppressLint("StaticFieldLeak")
    private static Vokaturi instance = null;

    private Vokaturi(Context context) {
        initialise();
        this.mContext = context;
    }

    private void initialise()
    {
        System.loadLibrary("vokaturi-lib");
    }

    /**
     * This method is used to instantiate the library and provide access to its functions.
     * @param context Application context
     * @return Instance of the library
     * @throws VokaturiException If dlopen() error occurs or Vokaturi library not found during the initialisation
     * @see VokaturiException#VOKATURI_ERROR
     */
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

    /**
     * Starts recording the audio from the device.
     * <p>The Vokaturi library currently supports only WAV format file to detect the emotions.
     * <p>The audio data gets recorded in a file created in cache directory of the application.
     * <p>You can retrieve the file using {@link #getRecordedAudio()}, but ONLY after calling {@link #stopListeningAndAnalyze()}.
     * @throws VokaturiException If audio recording permissions lack for the application
     * @see WavAudioRecorder
     * @see VokaturiException#VOKATURI_DENIED_PERMISSIONS
     */
    public synchronized void startListeningForSpeech() throws VokaturiException
    {
        if (recordingAudioState == RecordingAudioState.NOT_LISTENING) {
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
        } else {
            throw new IllegalStateException("Library is already listening for speech.");
        }
    }

    /**
     * Stops recording the audio, releases the cache file that was used by the {@link WavAudioRecorder} to record.
     * <p>And the file next is used by the library to analyze and detect the emotions from the audio data in the file.
     * <p> Call this only if you have called {@link #startListeningForSpeech()} before.
     * @return The metrics of the emotions that were processed by the library
     * @throws VokaturiException If there was any exception thrown in the native code while processing the file
     */
    public synchronized EmotionProbabilities stopListeningAndAnalyze() throws VokaturiException {
        if (recordingAudioState == RecordingAudioState.LISTENING) {
            stopRecordingWav();
            if(instance == null)
                throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "Vokaturi library has not been initialised");
            return analyzeForEmotions(mContext, FileToAnalyze);
        } else {
            throw new IllegalStateException("Cannot stop listening for already closed audio input.");
        }
    }

    /**
     * Gets the WAV file that contains the audio data which was used by the library to process.
     * @return The latest audio recording
     * @throws VokaturiException If there was no audio file ever processed before
     */
    public File getRecordedAudio() throws VokaturiException
    {
        if (FileToAnalyze == null)
        {
            throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "The library has not processed any audio file yet.");
        }
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
            recordingAudioState = RecordingAudioState.LISTENING;
            logD("WavAudioRecorder just started recording the audio");
        } else {
            logE("Some denied permissions found");
            throw new VokaturiException(VokaturiException.VOKATURI_DENIED_PERMISSIONS, "File Read Write/Recording Audio permissions not granted");
        }
    }

    private void stopRecordingWav() {
        if(recorder != null)
        {
            recordingAudioState = RecordingAudioState.NOT_LISTENING;
            recorder.stop();
            recorder.reset();
            recorder.release();
            recorder = null;
            logD("Stopped the WavAudioRecorder");
        }
    }

    private synchronized EmotionProbabilities analyzeForEmotions(Context context, String fileName) throws VokaturiException
    {
        if(shouldAskForPermissionsForGrants(context))
        {
            logE("Some denied permissions found");
            throw new VokaturiException(VokaturiException.VOKATURI_DENIED_PERMISSIONS, "File Read Write/Recording Audio permissions not granted");
        }
        logD("About to call native method #analyzeWav# to process the recorded file");

        return analyzeWav(fileName);
    }

    /**
     * Asynchronous method of analyzing an audio file for emotions.
     * <p>This does not block the GUI thread of the {@link android.app.Activity} while processing.
     * @param context Application context
     * @param fileName WAV format audio file to process
     * @param callback The result received after analyzing
     * @throws VokaturiException If the application permissions lack to access the file
     * @see VokaturiException#VOKATURI_DENIED_PERMISSIONS
     */
    public static void AnalyzeForEmotionsAsync(Context context, String fileName, VokaturiAsyncResult callback) throws VokaturiException
    {
        if(shouldAskForPermissionsForGrants(context))
            throw new VokaturiException(VokaturiException.VOKATURI_DENIED_PERMISSIONS, "File Read Write/Recording Audio permissions not granted");
        if (fileName != null) {
            if (!TextUtils.isEmpty(fileName)) {
                if(instance == null)
                    throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "Vokaturi library has not been initialised");
                new AsyncEmotionAnalyzerTask(callback).execute(fileName);
            } else {
                throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "Not a valid file name");
            }
        } else {
            throw new VokaturiException(VokaturiException.VOKATURI_ERROR, "Cannot pass a null reference to fileName");
        }
    }

    private static class AsyncEmotionAnalyzerTask extends AsyncTask<String, Void, Void>
    {
        private VokaturiAsyncResult resultCallback;

        AsyncEmotionAnalyzerTask(VokaturiAsyncResult callback)
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


    /**
     * Send debug log message
     * @param msg message
     */
    public static void logD(String msg){
        if (DEBUG_LOGGING_ENABLED) {
            Log.d(TAG, msg);
        }
    }

    /**
     * Send error log message
     * @param msg message
     */
    public static void logE(String msg){
        if (DEBUG_LOGGING_ENABLED) {
            Log.e(TAG, msg);
        }
    }

    /**
     * Short description of the version and license of the OpenVokaturi library
     * @return description
     */
    public static String versionAndLicense()
    {
        return "OpenVokaturi version 2.1 for open-source projects, 2017-01-13\n" +
                    "Distributed under the GNU General Public License, version 3 or later";
    }

    private static native EmotionProbabilities analyzeWav(String fileName) throws VokaturiException;

    private enum RecordingAudioState
    {
        LISTENING,
        NOT_LISTENING
    }
}
