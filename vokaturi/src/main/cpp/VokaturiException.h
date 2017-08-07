//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_VOKATURIEXCEPTION_H
#define VOKATURIANDROID_VOKATURIEXCEPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

void throwVokaturiException(
        JNIEnv *env,
        const char *functionName,
        const char *file,
        const int line,
        const int code,
        const char *message
);


void  throwVokaturiException(
        JNIEnv *env,
        const char *functionName,
        const char *file,
        const int line,
        const int code,
        const char *message
)
{
    // Find and instantiate a VokaturiException
    jclass exClass =
            env->FindClass ("com/projects/alshell/vokaturi/VokaturiException");

    jmethodID constructor =
            env->GetMethodID (exClass, "<init>",
                              "(ILjava/lang/String;)V");

    jobject exception = env->NewObject (exClass, constructor,
                                        code,
                                        env->NewStringUTF (message));


    // Find the __jni_setLocation method and call it with
    // the function name, file and line parameters
    jmethodID setLocation =
            env->GetMethodID (exClass, "__jni_setLocation",
                              "(Ljava/lang/String;"
                                      "Ljava/lang/String;"
                                      "I)V");

    env->CallVoidMethod (exception, setLocation,
                         env->NewStringUTF (functionName),
                         env->NewStringUTF (file),
                         line);

    // Throw the exception. Since this is native code,
    // execution continues, and the execution will be abruptly
    // interrupted at the point in time when we return to the VM.
    // The calling code will perform the early return back to Java code.
    env->Throw ((jthrowable) exception);

    // Clean up local reference
    env->DeleteLocalRef (exClass);
}

#define ThrowVokaturiException(code, message) throwVokaturiException(env, __func__, __FILE__, __LINE__, code, message)

#ifdef __cplusplus
}
#endif
#endif //VOKATURIANDROID_VOKATURIEXCEPTION_H
