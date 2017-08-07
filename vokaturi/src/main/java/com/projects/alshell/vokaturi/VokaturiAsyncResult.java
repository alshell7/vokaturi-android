package com.projects.alshell.vokaturi;

import android.content.Context;

/**
 * Callback made by the asynchronous method of analyzing for emotions from a file.
 * @see Vokaturi#AnalyzeForEmotionsAsync(Context, String, VokaturiAsyncResult)
 */

public interface VokaturiAsyncResult
{

    /**
     * If there were no exceptions thrown by the native code during the analysis for emotions
     * @param emotionProbabilities The metrics of the emotions that were processed
     */
    public void onSuccess(EmotionProbabilities emotionProbabilities);


    /**
     * If there was some problem that occurred while processing
     * @param e Exception describing the reason of the problem
     * @see VokaturiException#getErrorCode()
     */
    public void onError(VokaturiException e);
}
