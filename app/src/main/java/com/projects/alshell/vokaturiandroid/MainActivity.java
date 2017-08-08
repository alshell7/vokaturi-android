package com.projects.alshell.vokaturiandroid;

import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.projects.alshell.vokaturi.Emotion;
import com.projects.alshell.vokaturi.EmotionProbabilities;
import com.projects.alshell.vokaturi.Vokaturi;
import com.projects.alshell.vokaturi.VokaturiAsyncResult;
import com.projects.alshell.vokaturi.VokaturiException;

import static android.Manifest.permission.READ_EXTERNAL_STORAGE;
import static android.Manifest.permission.RECORD_AUDIO;
import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;
import static com.projects.alshell.vokaturi.Vokaturi.logD;

@SuppressWarnings("deprecation")
public class MainActivity extends AppCompatActivity
{

    private static final int PERMISSIONS_REQUEST_CODE = 5;

    private ImageView emojiEmotionImageView;
    private ProgressBar progressBarNeutrality;
    private ProgressBar progressBarHappiness;
    private ProgressBar progressBarSadness;
    private ProgressBar progressBarAnger;
    private ProgressBar progressBarFear;

    private TextView textViewNeutrality;
    private TextView textViewHappiness;
    private TextView textViewSadness;
    private TextView textViewAnger;
    private TextView textViewFear;

    private TextView actionStatus;

    private PlayPauseButton playPauseButton;

    private Vokaturi vokaturiApi;

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

        ActivityCompat.requestPermissions(MainActivity.this, new String[]{RECORD_AUDIO, WRITE_EXTERNAL_STORAGE}, PERMISSIONS_REQUEST_CODE);

        initializeViews();

        setListeners();
    }

    private void initializeViews()
    {
        emojiEmotionImageView = (ImageView) findViewById(R.id.emojiEmotionImageView);
        progressBarNeutrality = (ProgressBar) findViewById(R.id.progressBarNeutrality);
        progressBarHappiness = (ProgressBar) findViewById(R.id.progressBarHappiness);
        progressBarSadness = (ProgressBar) findViewById(R.id.progressBarSadness);
        progressBarAnger = (ProgressBar) findViewById(R.id.progressBarAnger);
        progressBarFear = (ProgressBar) findViewById(R.id.progressBarFear);
        playPauseButton = (PlayPauseButton) findViewById(R.id.playPauseButton);

        textViewNeutrality = (TextView) findViewById(R.id.textViewNeutrality);
        textViewHappiness = (TextView) findViewById(R.id.textViewHappiness);
        textViewSadness = (TextView) findViewById(R.id.textViewSadness);
        textViewAnger = (TextView) findViewById(R.id.textViewAnger);
        textViewFear = (TextView) findViewById(R.id.textViewFear);

        actionStatus = (TextView) findViewById(R.id.actionStatus);

    }

    private void setListeners()
    {
        playPauseButton.setOnControlStatusChangeListener(new PlayPauseButton.OnControlStatusChangeListener()
        {
            @Override
            public void onStatusChange(View view, boolean state)
            {
                if(state)
                {
                    startListening();
                } else {
                    stopListening();
                }
            }

        });
    }

    @SuppressLint("SetTextI18n")
    private void startListening()
    {
        if(vokaturiApi != null)
        {
            try {
                setListeningUI();
                vokaturiApi.startListeningForSpeech();
            } catch (VokaturiException e) {
                setNotListeningUI();
                if(e.getErrorCode() == VokaturiException.VOKATURI_DENIED_PERMISSIONS)
                {
                    Toast.makeText(this, "Grant Microphone and Storage permissions in the app settings to proceed", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(this, "There was some problem to start listening audio", Toast.LENGTH_SHORT).show();
                }
            } catch (IllegalStateException e)
            {
                setNotListeningUI();
                e.printStackTrace();
            }
        }
    }

    private void setListeningUI()
    {
        actionStatus.setText("Press again to stop listening and analyze emotions");
        progressBarNeutrality.setIndeterminate(true);
        progressBarHappiness.setIndeterminate(true);
        progressBarSadness.setIndeterminate(true);
        progressBarAnger.setIndeterminate(true);
        progressBarFear.setIndeterminate(true);
        emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_default));
    }

    @SuppressLint("SetTextI18n")
    private void stopListening()
    {
        if(vokaturiApi != null)
        {
            setNotListeningUI();

            try {
                showMetrics(vokaturiApi.stopListeningAndAnalyze());
            } catch (VokaturiException e) {
                if(e.getErrorCode() == VokaturiException.VOKATURI_NOT_ENOUGH_SONORANCY)
                {
                    Toast.makeText(this, "Please speak a more clear and avoid noise around your environment", Toast.LENGTH_LONG).show();
                }
            } catch (IllegalStateException e)
            {
                e.printStackTrace();
            }
        }
    }

    private void setNotListeningUI()
    {
        actionStatus.setText("Press below button to start listening");
        progressBarNeutrality.setIndeterminate(false);
        progressBarHappiness.setIndeterminate(false);
        progressBarSadness.setIndeterminate(false);
        progressBarAnger.setIndeterminate(false);
        progressBarFear.setIndeterminate(false);
    }


    @SuppressLint("SetTextI18n")
    private void showMetrics(EmotionProbabilities emotionProbabilities)
    {
        emotionProbabilities.scaledValues(5);

        logD("showMetrics for, " + emotionProbabilities.toString());
        textViewNeutrality.setText("Neutrality: " + emotionProbabilities.Neutrality);
        textViewHappiness.setText("Happiness: " + emotionProbabilities.Happiness);
        textViewSadness.setText("Sadness: " + emotionProbabilities.Sadness);
        textViewAnger.setText("Anger: " + emotionProbabilities.Anger);
        textViewFear.setText("Fear: " + emotionProbabilities.Fear);

        showEmojiBasedOnMetrics(emotionProbabilities);
        progressBarNeutrality.setProgress(normalizeForProgressBar(emotionProbabilities.Neutrality));
        progressBarHappiness.setProgress(normalizeForProgressBar(emotionProbabilities.Happiness));
        progressBarSadness.setProgress(normalizeForProgressBar(emotionProbabilities.Sadness));
        progressBarAnger.setProgress(normalizeForProgressBar(emotionProbabilities.Anger));
        progressBarFear.setProgress(normalizeForProgressBar(emotionProbabilities.Fear));

    }

    private int normalizeForProgressBar(double val)
    {
        if(val < 1)
        {
            return (int)(val * 100);
        } else {
            return (int)(val * 10);
        }
    }

    private void showEmojiBasedOnMetrics(EmotionProbabilities emotionProbabilities)
    {
        Emotion capturedEmotion = Vokaturi.extractEmotion(emotionProbabilities);
        if(capturedEmotion == Emotion.Neutral)
        {
            emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_neutral));
        } else if(capturedEmotion == Emotion.Happy)
        {
            emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_happiness));
        }
        else if(capturedEmotion == Emotion.Sad)
        {
            emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_sadness));
        }
        else if(capturedEmotion == Emotion.Angry)
        {
            emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_anger));
        }
        else if(capturedEmotion == Emotion.Feared)
        {
            emojiEmotionImageView.setImageDrawable(getResources().getDrawable(R.drawable.emoji_fear));
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);


        if (requestCode == PERMISSIONS_REQUEST_CODE)
        {
            if(grantResults[0] != PackageManager.PERMISSION_GRANTED)
            {
                Toast.makeText(this, "Audio recording permissions denied.", Toast.LENGTH_SHORT).show();
            }
        }

    }

}
