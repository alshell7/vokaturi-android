#include <jni.h>
#include "Vokaturi.h"
#include "WavFile.h"
#include "android/log.h"
#include "VokaturiException.h"

#define  DEBUG_LOG_TAG  "VokaturiLib"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, DEBUG_LOG_TAG, __VA_ARGS__)

VokaturiEmotionProbabilities perform(JNIEnv *env, const char *fileName);

/*
JavaVM* gJvm = nullptr;
static jobject gClassLoader;
static jmethodID gFindClassMethod;

*/

/*
JNIEnv* getEnv() {
    JNIEnv *env;
    LOGD(">> Fetching the *env");
    int status = gJvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if(status < 0) {
        status = gJvm->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            return nullptr;
        }
    }
    LOGD(">> Returning the jni *env");
    return env;
}

jclass findClass(const char* name) {
    LOGD(">> Finding class");
    return static_cast<jclass>(getEnv()->CallObjectMethod(gClassLoader, gFindClassMethod, getEnv()->NewStringUTF(name)));
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *pjvm, void *reserved) {

    LOGD(">> Vokaturi Loading..");
    gJvm = pjvm;

    auto env = getEnv();
    //replace with one of your classes in the line below
    auto randomClass = env->FindClass("com/projects/alshell/vokaturi/Vokaturi");
    jclass classClass = env->GetObjectClass(randomClass);
    auto classLoaderClass = env->FindClass("java/lang/ClassLoader");
    auto getClassLoaderMethod = env->GetMethodID(classClass, "getClassLoader",
                                                 "()Ljava/lang/ClassLoader;");
    gClassLoader = env->CallObjectMethod(randomClass, getClassLoaderMethod);
    gFindClassMethod = env->GetMethodID(classLoaderClass, "findClass",
                                        "(Ljava/lang/String;)Ljava/lang/Class;");

    return JNI_VERSION_1_6;
}
*/

void logD(JNIEnv *env, const char * msg, ...)
{
    jclass cls = env->FindClass("com/projects/alshell/vokaturi/Vokaturi");
    jfieldID fieldIsDebugEnabled = env->->GetFieldID(cls, "DEBUG_LOGGING_ENABLED", "Z");
    jboolean isDebugEnabled = env->->GetStaticBooleanField(cls, fieldIsDebugEnabled);
    if(isDebugEnabled == JNI_TRUE)
    {
        va_list ap;
        va_start(ap, fmt);
        __android_log_vprint(ANDROID_LOG_DEBUG, DEBUG_LOG_TAG, msg, ap);
        va_end(ap);
    }
}

void logE(JNIEnv *env, const char *msg, ...)
{
    jclass cls = env->FindClass("com/projects/alshell/vokaturi/Vokaturi");
    jfieldID fieldIsDebugEnabled = env->->GetFieldID(cls, "DEBUG_LOGGING_ENABLED", "Z");
    jboolean isDebugEnabled = env->->GetStaticBooleanField(cls, fieldIsDebugEnabled);
    if(isDebugEnabled == JNI_TRUE)
    {
        va_list ap;
        va_start(ap, fmt);
        __android_log_vprint(ANDROID_LOG_ERROR, DEBUG_LOG_TAG, msg, ap);
        va_end(ap);
    }
}


VokaturiEmotionProbabilities perform(JNIEnv *env, const char *fileName){

    LOGD(">>Vokaturi, Emotion analysis about to begin on the file");
    //LOGD(fileName);

    VokaturiWavFile wavFile;
    VokaturiWavFile_open (fileName, & wavFile);
    LOGD(">>@Vokaturi, File Open Successful");
    if (! VokaturiWavFile_valid (& wavFile)) {
        LOGE(">> Exception encountered. Invalid WAV file");
        VokaturiEmotionProbabilities emotionsNULL = VokaturiEmotionProbabilities();
        ThrowVokaturiException(-1, "Could not analyze WAV file. It seems the given file is not of a valid WAV format\n");
        return emotionsNULL;
    }

    //env->ExceptionClear();

    LOGD(">>@Vokaturi, Getting sample rate and buffer length of the WAV file");
    VokaturiVoice voice = VokaturiVoice_create (
            wavFile.samplingFrequency,
            wavFile.numberOfSamples
    );

    LOGD(">>@Vokaturi, Reading file to cache and filling voice on given samples");

    VokaturiWavFile_fillVoice (& wavFile, voice,
                               0,   // the only or left channel
                               0,   // starting from the first sample
                               wavFile.numberOfSamples   // all samples
    );

    VokaturiQuality quality;
    VokaturiEmotionProbabilities emotionProbabilities;

    LOGD(">>@Vokaturi, Extracting emotions now");
    VokaturiVoice_extract (voice, & quality, & emotionProbabilities);

    //char s[50];

    if (quality.valid) {
        LOGD (">>@Vokaturi, Neutrality : %f", emotionProbabilities.neutrality);

        //sprintf(s,"%f", emotionProbabilities.neutrality);
        //LOGD(s);

        LOGD (">>@Vokaturi, Happiness : %f", emotionProbabilities.happiness);
        //sprintf(s,"%f", emotionProbabilities.happiness);
        //LOGD(s);

        LOGD (">>@Vokaturi, Sadness : %f", emotionProbabilities.sadness);
        //sprintf(s,"%f", emotionProbabilities.sadness);
        //LOGD(s);

        LOGD (">>@Vokaturi, Anger : %f", emotionProbabilities.anger);
        //sprintf(s,"%f", emotionProbabilities.anger);
        //LOGD(s);


        LOGD (">>@Vokaturi, Fear : %f", emotionProbabilities.fear);
        //sprintf(s,"%f", emotionProbabilities.fear);
        //LOGD(s);

    } else {
        LOGE(">>@Vokaturi, Not enough sonorancy to determine emotions\n");
        /*emotionProbabilities.happiness = 0.0;
        emotionProbabilities.neutrality = 0.0;
        emotionProbabilities.fear = 0.0;
        emotionProbabilities.anger = 0.0;
        emotionProbabilities.sadness = 0.0;*/
        //com.projects.alshell.vokaturi.VokaturiException.VOKATURI_NOT_ENOUGH_SONORANCY = 2
        ThrowVokaturiException(2, "Not enough sonorancy to determine emotions\n");
    }

    LOGD(">>@Vokaturi Finishing analyze process");
    VokaturiVoice_destroy (voice);
    VokaturiWavFile_clear (& wavFile);

    LOGD(">>@Vokaturi Returning the results to JNI");
    return emotionProbabilities;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_projects_alshell_vokaturi_Vokaturi_analyzeWav(JNIEnv *env, jobject instance,
                                                       jstring fileName_){
    const char *fileName = env->GetStringUTFChars(fileName_, 0);

    VokaturiEmotionProbabilities probabilities = perform(env, fileName);

    if(env->ExceptionCheck())
    {
        LOGE(">>@JNI, There are few pending exceptions to take over");
        jthrowable exception = env->ExceptionOccurred();
        env->ExceptionClear();
        env->Throw(exception);
        return nullptr;
    } else {
        LOGD(">>@JNI, Instantiating EmotionProbabilities class in JNI");
        //jclass cls = env->FindClass("com/projects/alshell/vokaturi/EmotionProbabilities");
        jclass cls = env->FindClass("com/projects/alshell/vokaturi/EmotionProbabilities");
        //(Lhelloworld$ExportedFuncs;)V for inner classes
        jmethodID methodId = env->GetMethodID(cls, "<init>", "()V");

        LOGD(">>@JNI, Creating instance of the class");
        jobject obj = env->NewObject(cls, methodId);
        jclass emotionProbabilitiesClass = env->GetObjectClass(obj);

        LOGD(">>@JNI, Setting fields of EmotionProbabilities");
        jfieldID fieldIdNeutrality = env->GetFieldID(emotionProbabilitiesClass, "Neutrality", "D");
        jdouble jValueNeutrality = (jdouble) probabilities.neutrality;
        env->SetDoubleField(obj, fieldIdNeutrality, jValueNeutrality);
        LOGD (">>@JNI, Neutrality : %f", jValueNeutrality);

        jfieldID fieldIdHappiness = env->GetFieldID(emotionProbabilitiesClass, "Happiness", "D");
        jdouble jValueHappiness = (jdouble) probabilities.happiness;
        env->SetDoubleField(obj, fieldIdHappiness, jValueHappiness);
        LOGD (">>@JNI, Happiness : %f", jValueHappiness);

        jfieldID fieldIdSadness = env->GetFieldID(emotionProbabilitiesClass, "Sadness", "D");
        jdouble jValueSadness = (jdouble) probabilities.sadness;
        env->SetDoubleField(obj, fieldIdSadness, jValueSadness);
        LOGD (">>@JNI, Sadness : %f", jValueSadness);

        jfieldID fieldIdAnger = env->GetFieldID(emotionProbabilitiesClass, "Anger", "D");
        jdouble jValueAnger = (jdouble) probabilities.anger;
        env->SetDoubleField(obj, fieldIdAnger, jValueAnger);
        LOGD (">>@JNI, Anger : %f", jValueAnger);

        jfieldID fieldIdFear = env->GetFieldID(emotionProbabilitiesClass, "Fear", "D");
        jdouble jValueFear = (jdouble) probabilities.fear;
        env->SetDoubleField(obj, fieldIdFear, jValueFear);
        LOGD (">>@JNI, Fear : %f", jValueFear);

        //env->ReleaseStringUTFChars(fileName_, fileName);

        LOGD(">>@JNI, All Fields set");

        logD(env, "OK %f");
        LOGD(">>@JNI, Returning the EmotionProbabilities object to Java");
        return obj;
    }

}

