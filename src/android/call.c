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

// Android call using Intent

#ifndef HAL_NO_CALL
#include "hal/call.h"
#include <jni.h>
#include <stdlib.h>
#include <string.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

bool hal_call_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

static bool open_call_intent(const char *tel, const char *action) {
    if (tel == NULL) return false;
    
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
    jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
    
    // Create URI: tel:number
    char uri_str[128];
    snprintf(uri_str, sizeof(uri_str), "tel:%s", tel);
    jstring juri = (*env)->NewStringUTF(env, uri_str);
    
    jmethodID parseMethod = (*env)->GetStaticMethodID(env, uriClass, "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;");
    jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, parseMethod, juri);
    
    // Create intent
    jstring jaction = (*env)->NewStringUTF(env, action);
    jmethodID intentInit = (*env)->GetMethodID(env, intentClass, "<init>",
        "(Ljava/lang/String;Landroid/net/Uri;)V");
    jobject intent = (*env)->NewObject(env, intentClass, intentInit, jaction, uri);
    
    // Start activity
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity",
        "(Landroid/content/Intent;)V");
    (*env)->CallVoidMethod(env, g_activity, startActivity, intent);
    
    return true;
}

bool hal_call_dial(const char *tel) {
    return open_call_intent(tel, "android.intent.action.DIAL");
}

bool hal_call_make(const char *tel) {
    return open_call_intent(tel, "android.intent.action.CALL");
}

#endif // HAL_NO_CALL
