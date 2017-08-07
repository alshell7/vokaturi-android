package com.projects.alshell.vokaturi;

/**
 * Created by Alshell7 @(Ashraf Khan Workstation)
 * 09:01 PM.
 * 05/Aug/2017
 */

public interface VokaturiResultCallback
{
    public void onSuccess(EmotionProbabilities emotionProbabilities);
    public void onError(VokaturiException e);
}
