//
// Created by Ashraf Khan on 05/Aug/2017.
//


#include "Vokaturi9.h"
#include "Network9-100-20.h"
#include "VokaturiQuality.h"

#include "flint.h"
#include "Sound_and_Spectrum.h"
#include "Sound_and_Pitch.h"

#include <math.h>

const char *Vokaturi_versionAndLicense () {
    return
            "OpenVokaturi version 2.1 for open-source projects, 2017-01-13\n"
                    "Distributed under the GNU General Public License, version 3 or later";
}

struct structVokaturiVoice {
    double sampleRate;
    int bufferLength;
    double *buffer;
    flint numberOfFedSamples, numberOfAnalyzedSamples;
    int frameLength, frameStep, spectrumLength;
    Sound frame, autocorrelation;
    Spectrum spectrum;
    Pitch pitch;
    struct {
        Sound shape;
        double sumOfSquares;
        Sound autocorrelation;
    } window;
    struct {
        flint frameNumber;
        bool isSonorant;
        double pitch;
        double intensity;
        double spectralSlope;
    } leftleft, left, mid, right, rightright;
    flint numberOfFedFrames, numberOfAnalyzedFrames;
    EmotionProbabilities relativePriorEmotionProbabilities;
    double *hidden1, *hidden2;
};

VokaturiVoice VokaturiVoice_create (double sampleRate, int bufferLength) {
    VokaturiVoice me = NULL;

    if (sampleRate < 8000.0)
        goto error;
    if (bufferLength < 1)
        goto error;
    me = (VokaturiVoice) calloc (1, sizeof * me);
    if (me == NULL)
        goto error;
    my sampleRate = sampleRate;
    my buffer = (double *) calloc (bufferLength, sizeof * my buffer);
    if (my buffer == NULL) {
        goto error;
    }
    my bufferLength = bufferLength;
    /*
        A "frame" is a buffer that can hold at least 40 milliseconds of speech.
        Its physical length (number of samples) has to be a power of 2,
        so that we can do a fast Fourier transform on it.
        If the minimum sampling frequency is 8000 Hz,
        the frame length will be at least 320 samples.
    */
    {// scope because of gotos
        const double minimumFrameDuration_seconds = 0.04;
        my frameLength = (int) (minimumFrameDuration_seconds * my sampleRate);
    }
    {// scope because of gotos
        const int overSamplingFactor = 4;
        my frameStep = my frameLength / overSamplingFactor;
    }

    /*
        Make room for the frame.
    */
    Sound_initWithLengthAndSamplingFrequency (& my frame, my frameLength, my sampleRate);
    if (! Sound_isValid (& my frame)) {
        goto error;
    }

    /*
        Make room for the spectrum (reusable).
    */
    my spectrumLength = Sound_getBestSpectrumLength (& my frame);

    {// scope because of gotos
        const double frequencyStepInHz = sampleRate / my spectrumLength;
        Spectrum_initWithLengthAndFrequencyStep (& my spectrum, my spectrumLength, frequencyStepInHz);
    }
    if (! Spectrum_isValid (& my spectrum)) {
        goto error;
    }

    /*
        Make room for the analysis window, and compute it.
    */
    Sound_initWithLengthAndSamplingFrequency (& my window.shape, my frameLength, my sampleRate);
    if (! Sound_isValid (& my window.shape)) {
        goto error;
    }
    Sound_fillWithNuttallWindow (& my window.shape);
    my window.sumOfSquares = Sound_getSumOfSquares (& my window.shape);

    /*
        Make room for the autocorrelation of the window, and compute it.
    */
    Sound_initWithLengthAndSamplingFrequency (& my window.autocorrelation, my frameLength, my sampleRate);
    if (! Sound_isValid (& my window.autocorrelation)) {
        goto error;
    }
    Sound_into_Spectrum (& my window.shape, & my spectrum);
    Spectrum_square (& my spectrum);
    Spectrum_into_Sound (& my spectrum, & my window.autocorrelation);

    /*
        Make room for the autocorrelation of the sound.
    */
    Sound_initWithLengthAndSamplingFrequency (& my autocorrelation, my frameLength, my sampleRate);
    if (! Sound_isValid (& my autocorrelation)) {
        goto error;
    }

    {// scope because of gotos
        VokaturiEmotionProbabilities priors = { 1.0, 1.0, 1.0, 1.0, 1.0 };
        VokaturiVoice_setRelativePriorProbabilities (me, & priors);
    }

    my hidden1 = (double *) calloc (NUMBER_OF_HIDDEN1, sizeof * my hidden1);
    if (my hidden1 == NULL) {
        goto error;
    }
    my hidden2 = (double *) calloc (NUMBER_OF_HIDDEN2, sizeof * my hidden2);
    if (my hidden2 == NULL) {
        goto error;
    }

    return me;
    error:
    VokaturiVoice_destroy (me);
    return NULL;
}

void VokaturiVoice_setRelativePriorProbabilities (VokaturiVoice me,
                                                  VokaturiEmotionProbabilities *priorEmotionProbabilities)
{
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        my relativePriorEmotionProbabilities [emotion] = ((double *) priorEmotionProbabilities) [emotion];   // yucky cast
    }
}

void VokaturiVoice_fill (VokaturiVoice me, int num_samples, double samples []) {
    int buffer_index = flint_mod (my numberOfFedSamples, my bufferLength);
    for (int isamp = 0; isamp < num_samples; isamp ++) {
        my buffer [buffer_index] = samples [isamp];
        buffer_index ++;
        if (buffer_index == my bufferLength) {
            buffer_index = 0;
        }
    }
    my numberOfFedSamples += num_samples;
    //fprintf (stderr, "samples fed %ld analyzed %ld\n", (long) voice -> numberOfFedSamples, (long) voice -> numberOfAnalyzedSamples);
}

void VokaturiVoice_extractCues9 (VokaturiVoice me, VokaturiQuality *quality, CueStrengths9 cueStrengths) {
    if (quality == NULL) return;
    /*
        In case we don't have enough voiced data to analyze,
        we'll have to notify the VokaturiAnalysis,
        and we do that by reporting the number of voiced frames successfully analyzed.
        As we could notice the lack of data at any point during analysis,
        we set the notification correctly now,
        and update it only after we are sure that some frames have been analyzed.
    */
    int numberOfFramesAnalyzed = 0;
    /*
        As in Praat, we only consider complete data frames,
        i.e. we don't fill partial frames with zeroes.
    */
    if (my numberOfFedSamples < my frameLength)
        return VokaturiQuality_error (quality);

    flint oldNumberOfAnalyzedFrames = my numberOfAnalyzedFrames;
    flint newNumberOfAnalyzedFrames = flint_div (my numberOfFedSamples - my frameLength, my frameStep) + 1;
    int numberOfFramesToAnalyze = newNumberOfAnalyzedFrames - oldNumberOfAnalyzedFrames;
    if (numberOfFramesToAnalyze <= 0)
        return VokaturiQuality_error (quality);
    flint newNumberOfAnalyzedSamples = my frameLength + (newNumberOfAnalyzedFrames - 1) * my frameStep;
    //assert (newNumberOfAnalyzedSamples <= my numberOfFedSamples);
    flint sourceStartSample = newNumberOfAnalyzedSamples -
                              (my frameLength + my frameStep * (numberOfFramesToAnalyze - 1));
    int sourceStartIndex = flint_mod (sourceStartSample, my bufferLength);

    for (int cue = 0; cue < NUMBER_OF_CUES9; cue ++)
        cueStrengths [cue] = 0.0;
    for (int iframe = 0; iframe < numberOfFramesToAnalyze; iframe ++) {
        my leftleft = my left;
        my left = my mid;
        my mid = my right;
        my right = my rightright;
        /*
            We are going to analyze a frame.
            Start by copying samples from the ring buffer into the frame buffer.
        */
        int sourceSampleIndex = sourceStartIndex;
        for (int isamp = 0; isamp < my frameLength; isamp ++) {
            my frame.samples [isamp] = my buffer [sourceSampleIndex];
            sourceSampleIndex ++;
            if (sourceSampleIndex == my bufferLength)
                sourceSampleIndex = 0;
        }
        sourceStartIndex += my frameStep;
        if (sourceStartIndex >= my bufferLength) {
            sourceStartIndex -= my bufferLength;
        }

        /*
            The microphone or sound card may have a DC offset.
            We get rid of it.
        */
        Sound_subtractMean (& my frame);

        /*
            We apply an analysis window for two reasons:
            - accurate measurement of spectral content cannot stand abrupt cut-off at the edges;
            - the middle of the frame is more important than the edges anyway.
        */
        Sound_multiplyByOtherSound (& my frame, & my window.shape);

        /*
            Compute the average intensity of this frame.
        */
        double sumOfSquares = Sound_getSumOfSquares (& my frame);
        double meanOfSquares = sumOfSquares / my window.sumOfSquares;
        /*
            Just as in Praat, we assume that the mean-of-squares is actually
            the true intensity expressed in pascal-squared.
            Of course the correctness of this assumption depends on correct calibration,
            and it is unlikely that incoming sounds have been calibrated.
            Nevertheless, if your incoming sounds have their samples calibrated in pascal,
            the current procedure will correctly compute the intensity in dB
            with respect to the auditory threshold,
            which is standardly defined as 2e-5 Pa.
            See http://www.fon.hum.uva.nl/praat/manual/sound_pressure_calibration.html
        */
        const double auditoryThreshold_pascal = 2e-5;
        const double auditoryThreshold_pascalSquared = auditoryThreshold_pascal * auditoryThreshold_pascal;
        double intensity_dB = 10.0 * log10 (meanOfSquares / auditoryThreshold_pascalSquared);   // by definition
        my rightright.intensity = intensity_dB;

        /*
            Compute the spectrum of this frame.
        */
        Sound_into_Spectrum (& my frame, & my spectrum);

        /*
            Compute the energies below and above 1000 Hz.
        */
        double energyDensityInLowBand =
                Spectrum_computeBandEnergyDensity (& my spectrum, 50.0, 1000.0);
        double energyDensityInHighBand =
                Spectrum_computeBandEnergyDensity (& my spectrum, 1000.0, 4000.0);
        energyDensityInLowBand = 10.0 * log10 (energyDensityInLowBand);
        energyDensityInHighBand = 10.0 * log10 (energyDensityInHighBand);
        my rightright.spectralSlope = energyDensityInHighBand - energyDensityInLowBand;

        /*
            Compute the autocorrelation of the sound.
        */
        Spectrum_square (& my spectrum);
        Spectrum_into_Sound (& my spectrum, & my autocorrelation);

        /*
            Divide the autocorrelation of the windowed sound by the autocorrelation of the window.
        */
        Sound_divideByOtherSound (& my autocorrelation, & my window.autocorrelation);

        /*
            Look for the maximum autocorrelation in the range.
        */
        Sound_autocorrelation_into_Pitch (& my autocorrelation, & my pitch, 75.0, 600.0);
        my rightright.pitch = Pitch_getValueInSemitonesRe100Hz (& my pitch);

        my rightright.isSonorant = ( my rightright.intensity > 55.0 && my pitch.strength > 0.45 );
        //fprintf (stderr, "strength %f pitch %f\n", strength, my rightright.pitch);
        my rightright.frameNumber = oldNumberOfAnalyzedFrames + iframe;
        if (my leftleft.isSonorant &&
            my left.isSonorant &&
            my mid.isSonorant &&
            my right.isSonorant &&
            my rightright.isSonorant &&
            my left.frameNumber == my leftleft.frameNumber + 1 &&
            my mid.frameNumber == my left.frameNumber + 1 &&
            my right.frameNumber == my mid.frameNumber + 1 &&
            my rightright.frameNumber == my right.frameNumber + 1
                )
        {
            cueStrengths [CUE_pitAve] += my mid.pitch;
            cueStrengths [CUE_intAve] += my mid.intensity;
            cueStrengths [CUE_spcSlo] += my mid.spectralSlope;
            double dt = my frameStep / my sampleRate;
            double dpitch1 = fabs (my mid.pitch - my left.pitch);
            dpitch1 = flint_mod (dpitch1, 12.0);
            if (dpitch1 > 6.0) dpitch1 = 12.0 - dpitch1;
            double dpitch2 = fabs (my right.pitch - my mid.pitch);
            dpitch2 = flint_mod (dpitch2, 12.0);
            if (dpitch2 > 6.0) dpitch2 = 12.0 - dpitch2;
            cueStrengths [CUE_pitDyn] += 0.5 * (dpitch1 + dpitch2) / dt;
            double pitchDirection = (my right.pitch - my left.pitch) / dt;
            cueStrengths [CUE_pitDir] +=
                    pitchDirection > 0.0 ? pow (pitchDirection, 0.3333) : - pow (- pitchDirection, 0.3333);
            //fprintf (stderr, "pitch %f %f %f\n", my previous.pitch, my current.pitch, dt);
            cueStrengths [CUE_intDyn] += fabs (my right.intensity - my left.intensity) / 2.0 / dt;
            cueStrengths [CUE_pitJit] += fabs (2 * my mid.pitch - my left.pitch - my right.pitch) / dt;
            cueStrengths [CUE_intJit] += fabs (2 * my mid.intensity - my left.intensity - my right.intensity) / dt;
            cueStrengths [CUE_spcJit] += fabs (2 * my mid.spectralSlope - my left.spectralSlope - my right.spectralSlope) / dt;
            numberOfFramesAnalyzed ++;
        }
    }

    /*
        Notify the VokaturiVoice that we have analyzed some new samples.
    */
    my numberOfAnalyzedSamples = newNumberOfAnalyzedSamples;
    my numberOfAnalyzedFrames = newNumberOfAnalyzedFrames;

    if (numberOfFramesAnalyzed <= 0)
        return VokaturiQuality_error (quality);

    /*
        Output: the quality and the average strengths of the nine cues.
    */
    quality -> valid = true;
    quality -> num_frames_analyzed = numberOfFramesAnalyzed;
    quality -> num_frames_lost = 0;
    for (int cue = 0; cue < NUMBER_OF_CUES9; cue ++)
        cueStrengths [cue] /= numberOfFramesAnalyzed;
}

void VokaturiVoice_extract (VokaturiVoice me,
                            VokaturiQuality *quality,
                            VokaturiEmotionProbabilities *emotionProbabilities)
{
    if (quality == NULL) return;
    if (emotionProbabilities == NULL) return VokaturiQuality_error (quality);
    CueStrengths9 cueStrengths;
    VokaturiVoice_extractCues9 (me, quality, cueStrengths);
    if (! quality -> valid) return;

    /*
        Output: the emotion probabilities.
        We compute the emotion probabilities via a neural network
        with three levels of linear connections.
        The two hidden layers of nodes consist of rectifying units.
        The network was trained on EmoDB and Savee (see Train9.c);
        for the resulting parameters, see Network9-100-20.h.

        The input to the network consists of nine nodes that contain the strengths
        of the nine cues, converted into something similar to z values
        (this is the same transformation that was used in training,
        where it speeds up learning appreciably).
    */
    double input [NUMBER_OF_CUES9];
    for (int cue = 0; cue < NUMBER_OF_CUES9; cue ++)
        input [cue] = (cueStrengths [cue] - Cues9_mean [cue]) / Cues9_stdev [cue];   // conversion to z values
    /*
        Information proceeds toward to the first layer of hidden nodes.
        Each of the 100 nodes has a bias, as well as a weight to each of the nine input nodes.
    */
    for (int ihidden1 = 0; ihidden1 < NUMBER_OF_HIDDEN1; ihidden1 ++) {
        my hidden1 [ihidden1] = bias1 [ihidden1];
        for (int cue = 0; cue < NUMBER_OF_CUES9; cue ++) {
            my hidden1 [ihidden1] += input [cue] * weight1 [cue] [ihidden1];
        }
        if (my hidden1 [ihidden1] < 0.0)   // rectify
            my hidden1 [ihidden1] = 0.0;
    }
    /*
        Information then proceeds toward to the second layer of hidden nodes.
        Each of the 20 nodes has a bias, as well as a weight to each of the 100 lower nodes.
    */
    for (int ihidden2 = 0; ihidden2 < NUMBER_OF_HIDDEN2; ihidden2 ++) {
        my hidden2 [ihidden2] = bias3 [ihidden2];
        for (int ihidden1 = 0; ihidden1 < NUMBER_OF_HIDDEN1; ihidden1 ++) {
            my hidden2 [ihidden2] += my hidden1 [ihidden1] * weight3 [ihidden1] [ihidden2];
        }
        if (my hidden2 [ihidden2] < 0.0)   // rectify
            my hidden2 [ihidden2] = 0.0;
    }
    /*
        Information then proceeds toward to the output layer, which contains five nodes,
        i.e. one node for each emotion class.
        Each of the 5 nodes has a bias, as well as a weight to each of the 20 lower nodes.
    */
    double output [NUMBER_OF_EMOTIONS];
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        output [emotion] = bias5 [emotion];
        for (int ihidden2 = 0; ihidden2 < NUMBER_OF_HIDDEN2; ihidden2 ++) {
            output [emotion] += my hidden2 [ihidden2] * weight5 [ihidden2] [emotion];
        }
    }
    /*
        Turning the output activities into probabilities requires a softmax transformation:
        the probability of a class is proportional to its exponentiated output value.

        The first softmax step is therefore the exponentiation of the outputs.
        To prevent overflow or underflow, we have to first shift the maximum output to zero.
    */
    double maximum = -1e308;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)   // get the maximum output
        if (output [emotion] > maximum)
            maximum = output [emotion];
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)   // shift outputs to nonpositive values
        output [emotion] -= maximum;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)   // exponentiate
        output [emotion] = exp (output [emotion]);
    /*
        Now that the outputs have been converted into relative probabilities,
        we can multiply them by the prior probabilities, which are 1 by default.
    */
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)
        output [emotion] *= my relativePriorEmotionProbabilities [emotion];
    /*
        The final step, required by both softmax and the weighting,
        is to scale the outputs so that they sum to one.
    */
    double sum = 0.0;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)
        sum += output [emotion];
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++)
        output [emotion] /= sum;

    emotionProbabilities -> neutrality = output [EMOTION_Neu];
    emotionProbabilities -> happiness  = output [EMOTION_Hap];
    emotionProbabilities -> sadness    = output [EMOTION_Sad];
    emotionProbabilities -> anger      = output [EMOTION_Ang];
    emotionProbabilities -> fear       = output [EMOTION_Fea];
}

void VokaturiVoice_destroy (VokaturiVoice me) {
    if (me) {
        if (my buffer) free (my buffer);
        Sound_destroy (& my frame);
        Spectrum_destroy (& my spectrum);
        Sound_destroy (& my window.shape);
        Sound_destroy (& my window.autocorrelation);
        Sound_destroy (& my autocorrelation);
        if (my hidden1) free (my hidden1);
        if (my hidden2) free (my hidden2);
        free (me);
    }
}

void VokaturiVoice_reset (VokaturiVoice me) {
    my numberOfFedSamples = 0;
    my numberOfAnalyzedSamples = 0;
    my leftleft.frameNumber = 0;
    my leftleft.isSonorant = false;
    my left.frameNumber = 0;
    my left.isSonorant = false;
    my mid.frameNumber = 0;
    my mid.isSonorant = false;
    my right.frameNumber = 0;
    my right.isSonorant = false;
    my rightright.frameNumber = 0;
    my rightright.isSonorant = false;
    my numberOfFedFrames = 0;
    my numberOfAnalyzedFrames = 0;
}
