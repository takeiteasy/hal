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

// Android unique ID using Settings.Secure.ANDROID_ID

#ifndef HAL_NO_UNIQUE_ID
#include "hal/unique_id.h"
#include <jni.h>
#include <string.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static char unique_id_buffer[256] = {0};

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

bool hal_unique_id_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

const char* hal_unique_id_get(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return NULL;
    
    // Get content resolver
    jclass activity_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_resolver = (*env)->GetMethodID(env, activity_class, 
        "getContentResolver", "()Landroid/content/ContentResolver;");
    jobject resolver = (*env)->CallObjectMethod(env, g_activity, get_resolver);
    if (!resolver)
        return NULL;
    
    // Get Settings.Secure class
    jclass secure_class = (*env)->FindClass(env, "android/provider/Settings$Secure");
    if (!secure_class)
        return NULL;
    
    // Get ANDROID_ID constant
    jfieldID android_id_field = (*env)->GetStaticFieldID(env, secure_class, "ANDROID_ID", "Ljava/lang/String;");
    jstring android_id_key = (jstring)(*env)->GetStaticObjectField(env, secure_class, android_id_field);
    
    // Call getString
    jmethodID get_string = (*env)->GetStaticMethodID(env, secure_class, 
        "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
    jstring result = (jstring)(*env)->CallStaticObjectMethod(env, secure_class, get_string, resolver, android_id_key);
    
    if (result) {
        const char *str = (*env)->GetStringUTFChars(env, result, NULL);
        if (str) {
            strncpy(unique_id_buffer, str, sizeof(unique_id_buffer) - 1);
            unique_id_buffer[sizeof(unique_id_buffer) - 1] = '\0';
            (*env)->ReleaseStringUTFChars(env, result, str);
        }
        (*env)->DeleteLocalRef(env, result);
    }
    
    (*env)->DeleteLocalRef(env, resolver);
    
    return unique_id_buffer[0] ? unique_id_buffer : NULL;
}
#endif // HAL_NO_UNIQUE_ID
