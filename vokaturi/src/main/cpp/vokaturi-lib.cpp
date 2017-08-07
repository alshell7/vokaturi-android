#include <jni.h>
#include "Vokaturi.h"
#include "WavFile.h"
#include "android/log.h"
#include "VokaturiException.h"

#define  DEBUG_LOG_TAG  "VokaturiLib"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, DEBUG_LOG_TAG, __VA_ARGS__)

VokaturiEmotionProbabilities processFile(JNIEnv *env, const char *fileName);

void logD(JNIEnv *env, const char * msg, ...)
{
    if (env->ExceptionCheck() == JNI_FALSE) {
        jclass cls = env->FindClass("com/projects/alshell/vokaturi/Vokaturi");
        jfieldID fieldIsDebugEnabled = env->GetStaticFieldID(cls, "DEBUG_LOGGING_ENABLED", "Z");
        jboolean isDebugEnabled = env->GetStaticBooleanField(cls, fieldIsDebugEnabled);
        if(isDebugEnabled == JNI_TRUE)
        {
            va_list ap;
            va_start(ap, msg);
            __android_log_vprint(ANDROID_LOG_DEBUG, DEBUG_LOG_TAG, msg, ap);
            va_end(ap);
        }
    }
}

void logE(JNIEnv *env, const char *msg, ...)
{
    if (env->ExceptionCheck() == JNI_FALSE) {
        jclass cls = env->FindClass("com/projects/alshell/vokaturi/Vokaturi");
        jfieldID fieldIsDebugEnabled = env->GetStaticFieldID(cls, "DEBUG_LOGGING_ENABLED", "Z");
        jboolean isDebugEnabled = env->GetStaticBooleanField(cls, fieldIsDebugEnabled);
        if(isDebugEnabled == JNI_TRUE)
        {
            va_list ap;
            va_start(ap, msg);
            __android_log_vprint(ANDROID_LOG_ERROR, DEBUG_LOG_TAG, msg, ap);
            va_end(ap);
        }
    }
}

VokaturiEmotionProbabilities processFile(JNIEnv *env, const char *fileName){

    logD(env, ">>@Vokaturi, Emotion analysis about to begin on the file");

    VokaturiWavFile wavFile;

    VokaturiWavFile_open (fileName, & wavFile);
    logD(env, ">>@Vokaturi, File Open Successful");

    if (! VokaturiWavFile_valid (& wavFile)) {
        logE(env, ">>@Vokaturi, Exception encountered. Invalid WAV file");
        VokaturiEmotionProbabilities emotionsNULL = VokaturiEmotionProbabilities();
        //com.projects.alshell.vokaturi.VokaturiException.VOKATURI_INVALID_WAV_FILE = 4;
        ThrowVokaturiException(4, "Could not analyze WAV file. It seems the given file is not of a valid WAV format\n");
        return emotionsNULL;
    }

    logD(env, ">>@Vokaturi, Getting sample rate and buffer length of the WAV file");
    VokaturiVoice voice = VokaturiVoice_create (wavFile.samplingFrequency, wavFile.numberOfSamples);

    logD(env, ">>@Vokaturi, Reading file to cache and filling voice on given samples");

    VokaturiWavFile_fillVoice (& wavFile, voice, 0, 0, wavFile.numberOfSamples);

    VokaturiQuality quality;
    VokaturiEmotionProbabilities emotionProbabilities;

    logD(env, ">>@Vokaturi, Extracting emotions now");
    VokaturiVoice_extract (voice, & quality, & emotionProbabilities);

    if (quality.valid) {
        logD(env, ">>@Vokaturi, Neutrality : %f", emotionProbabilities.neutrality);

        logD(env, ">>@Vokaturi, Happiness : %f", emotionProbabilities.happiness);

        logD(env, ">>@Vokaturi, Sadness : %f", emotionProbabilities.sadness);

        logD(env, ">>@Vokaturi, Anger : %f", emotionProbabilities.anger);

        logD(env, ">>@Vokaturi, Fear : %f", emotionProbabilities.fear);

    } else {
        logE(env, ">>@Vokaturi, Not enough sonorancy to determine emotions\n");

        //com.projects.alshell.vokaturi.VokaturiException.VOKATURI_NOT_ENOUGH_SONORANCY = 2
        ThrowVokaturiException(2, "Not enough sonorancy to determine emotions\n");
    }

    logD(env, ">>@Vokaturi, Finishing analyze process");
    VokaturiVoice_destroy (voice);
    VokaturiWavFile_clear (& wavFile);

    logD(env, ">>@Vokaturi, Returning the results to JNI");
    return emotionProbabilities;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_projects_alshell_vokaturi_Vokaturi_analyzeWav(JNIEnv *env, jobject instance,
                                                       jstring fileName_){
    const char *fileName = env->GetStringUTFChars(fileName_, 0);

    VokaturiEmotionProbabilities probabilities = processFile(env, fileName);

    logD(env, ">>@JNI, Results received from Vokaturi");

    if(env->ExceptionCheck())
    {
        LOGE(">>There are few pending exceptions to take over");
        jthrowable exception = env->ExceptionOccurred();
        env->ExceptionClear();
        env->Throw(exception);
        return nullptr;
    } else {
        logD(env, ">>@JNI, Instantiating EmotionProbabilities class in JNI");

        jclass cls = env->FindClass("com/projects/alshell/vokaturi/EmotionProbabilities");
        jmethodID methodId = env->GetMethodID(cls, "<init>", "()V");

        logD(env, ">>@JNI, Creating instance of the class");
        jobject obj = env->NewObject(cls, methodId);
        jclass emotionProbabilitiesClass = env->GetObjectClass(obj);

        logD(env, ">>@JNI, Setting fields of EmotionProbabilities");
        jfieldID fieldIdNeutrality = env->GetFieldID(emotionProbabilitiesClass, "Neutrality", "D");
        jdouble jValueNeutrality = (jdouble) probabilities.neutrality;
        env->SetDoubleField(obj, fieldIdNeutrality, jValueNeutrality);


        jfieldID fieldIdHappiness = env->GetFieldID(emotionProbabilitiesClass, "Happiness", "D");
        jdouble jValueHappiness = (jdouble) probabilities.happiness;
        env->SetDoubleField(obj, fieldIdHappiness, jValueHappiness);


        jfieldID fieldIdSadness = env->GetFieldID(emotionProbabilitiesClass, "Sadness", "D");
        jdouble jValueSadness = (jdouble) probabilities.sadness;
        env->SetDoubleField(obj, fieldIdSadness, jValueSadness);


        jfieldID fieldIdAnger = env->GetFieldID(emotionProbabilitiesClass, "Anger", "D");
        jdouble jValueAnger = (jdouble) probabilities.anger;
        env->SetDoubleField(obj, fieldIdAnger, jValueAnger);


        jfieldID fieldIdFear = env->GetFieldID(emotionProbabilitiesClass, "Fear", "D");
        jdouble jValueFear = (jdouble) probabilities.fear;
        env->SetDoubleField(obj, fieldIdFear, jValueFear);

        logD(env, ">>@JNI, All Fields set");

        logD(env, ">>@JNI, Returning the EmotionProbabilities object to Java");
        return obj;
    }

}

