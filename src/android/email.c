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

// Android email using Intent

#ifndef HAL_NO_EMAIL
#include "hal/email.h"
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

bool hal_email_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_email_send(const char *recipient, const char *subject,
                    const char *body, const char *cc, const char *bcc) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
    
    // Create SENDTO intent with mailto:
    jstring actionSendTo = (*env)->NewStringUTF(env, "android.intent.action.SENDTO");
    jmethodID intentInit = (*env)->GetMethodID(env, intentClass, "<init>", "(Ljava/lang/String;)V");
    jobject intent = (*env)->NewObject(env, intentClass, intentInit, actionSendTo);
    
    // Set mailto: URI
    jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
    jmethodID parseMethod = (*env)->GetStaticMethodID(env, uriClass, "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;");
    
    char mailto[256] = "mailto:";
    if (recipient) strncat(mailto, recipient, sizeof(mailto) - strlen(mailto) - 1);
    jstring jmailto = (*env)->NewStringUTF(env, mailto);
    jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, parseMethod, jmailto);
    
    jmethodID setData = (*env)->GetMethodID(env, intentClass, "setData",
        "(Landroid/net/Uri;)Landroid/content/Intent;");
    (*env)->CallObjectMethod(env, intent, setData, uri);
    
    // Set extras
    jmethodID putExtra = (*env)->GetMethodID(env, intentClass, "putExtra",
        "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;");
    
    if (subject) {
        jstring key = (*env)->NewStringUTF(env, "android.intent.extra.SUBJECT");
        jstring val = (*env)->NewStringUTF(env, subject);
        (*env)->CallObjectMethod(env, intent, putExtra, key, val);
    }
    if (body) {
        jstring key = (*env)->NewStringUTF(env, "android.intent.extra.TEXT");
        jstring val = (*env)->NewStringUTF(env, body);
        (*env)->CallObjectMethod(env, intent, putExtra, key, val);
    }
    
    // Start activity
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity",
        "(Landroid/content/Intent;)V");
    (*env)->CallVoidMethod(env, g_activity, startActivity, intent);
    
    return true;
}

#endif // HAL_NO_EMAIL
