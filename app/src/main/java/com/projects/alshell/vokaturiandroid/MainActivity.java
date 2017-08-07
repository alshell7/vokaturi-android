package com.projects.alshell.vokaturiandroid;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.projects.alshell.vokaturi.Vokaturi;
import com.projects.alshell.vokaturi.VokaturiException;

import java.io.IOException;

import static com.projects.alshell.vokaturi.Vokaturi.DEBUG_LOGGING_ENABLED;
import static com.projects.alshell.vokaturi.Vokaturi.logD;

public class MainActivity extends AppCompatActivity
{

    public static final String TAG = "MainActivity";

    Vokaturi vokaturiApi;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        try {
            logD("About to instantiate the library");
            vokaturiApi = Vokaturi.getInstance(MainActivity.this);
        } catch (VokaturiException e) {
            e.printStackTrace();
        }
    }

    public void onClick(View view)
    {
        if(vokaturiApi != null)
        {
            int id = view.getId();
            switch (id)
            {
                case R.id.buttonStartRecording:
                    try {
                        vokaturiApi.startRecordingAudio();
                    } catch (VokaturiException e) {
                        e.printStackTrace();
                    }
                    break;
                case R.id.buttonStopRecording:
                    try {
                        vokaturiApi.stopRecordingAndAnalyze();
                    } catch (VokaturiException e) {
                        if(e.getErrorCode() == VokaturiException.VOKATURI_NOT_ENOUGH_SONORANCY)
                        {
                            Toast.makeText(this, "Not enough sonorancy to determine emotions", Toast.LENGTH_SHORT).show();
                        }
                    }
                    break;
                case R.id.buttonPlayRecording:
                    new PlayRecordedAudioTask().execute(vokaturiApi.getRecordedAudio().getAbsolutePath());
                    break;
            }
        } else {
            logD("Vokaturi library not initialised");
        }
    }


    private class PlayRecordedAudioTask extends AsyncTask<String, Void, Void>
            implements MediaPlayer.OnPreparedListener
    {
        private MediaPlayer mediaPlayer;

        @Override
        protected Void doInBackground(String... params) {
            mediaPlayer = new MediaPlayer();
            try {
                mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
                mediaPlayer.setDataSource(params[0]);
                mediaPlayer.setOnPreparedListener(this);
                mediaPlayer.prepareAsync();
            } catch (IllegalArgumentException | IOException | IllegalStateException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
        }

        @Override
        public void onPrepared(MediaPlayer mp) {
            mediaPlayer.start();

        }
    }

    public static void logD(String msg){
        if (DEBUG_LOGGING_ENABLED) {
            Log.d(TAG, msg);
        }
    }

}
