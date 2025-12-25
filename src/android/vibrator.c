/* https://github.com/takeiteasy/hal

 hal Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

// Android vibrator using Vibrator service

#ifndef HAL_NO_VIBRATOR
#include "hal/vibrator.h"
#include <jni.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv* get_jni_env(void) {
    JNIEnv *env = NULL;
    if (g_jvm) {
        (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
        if (!env) {
            (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        }
    }
    return env;
}

static jobject get_vibrator(JNIEnv *env) {
    if (!env || !g_activity)
        return NULL;
    
    // Get Context.VIBRATOR_SERVICE
    jclass context_class = (*env)->FindClass(env, "android/content/Context");
    jfieldID vibrator_field = (*env)->GetStaticFieldID(env, context_class, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jstring vibrator_service = (jstring)(*env)->GetStaticObjectField(env, context_class, vibrator_field);
    
    // Get system service
    jmethodID get_service = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, g_activity),
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
    return (*env)->CallObjectMethod(env, g_activity, get_service, vibrator_service);
}

bool hal_vibrator_available(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return false;
    
    jobject vibrator = get_vibrator(env);
    if (!vibrator)
        return false;
    
    // Check hasVibrator() for API 11+
    jclass vibrator_class = (*env)->GetObjectClass(env, vibrator);
    jmethodID has_vibrator = (*env)->GetMethodID(env, vibrator_class, "hasVibrator", "()Z");
    jboolean result = (*env)->CallBooleanMethod(env, vibrator, has_vibrator);
    
    (*env)->DeleteLocalRef(env, vibrator);
    return result;
}

void hal_vibrator_vibrate(int duration_ms) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return;
    
    jobject vibrator = get_vibrator(env);
    if (!vibrator)
        return;
    
    jclass vibrator_class = (*env)->GetObjectClass(env, vibrator);
    jmethodID vibrate = (*env)->GetMethodID(env, vibrator_class, "vibrate", "(J)V");
    (*env)->CallVoidMethod(env, vibrator, vibrate, (jlong)duration_ms);
    
    (*env)->DeleteLocalRef(env, vibrator);
}

void hal_vibrator_cancel(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return;
    
    jobject vibrator = get_vibrator(env);
    if (!vibrator)
        return;
    
    jclass vibrator_class = (*env)->GetObjectClass(env, vibrator);
    jmethodID cancel = (*env)->GetMethodID(env, vibrator_class, "cancel", "()V");
    (*env)->CallVoidMethod(env, vibrator, cancel);
    
    (*env)->DeleteLocalRef(env, vibrator);
}
#endif // HAL_NO_VIBRATOR
