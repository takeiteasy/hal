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

// Android notifications using NotificationManager via JNI

#ifndef HAL_NO_NOTIFICATION
#include "hal/notification.h"
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

bool hal_notification_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_notification_send(const char *title, const char *message,
                           const char *app_name, int timeout_sec) {
    (void)app_name; (void)timeout_sec;
    
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return false;
    
    // Get NotificationManager
    jclass contextClass = (*env)->FindClass(env, "android/content/Context");
    jfieldID nsField = (*env)->GetStaticFieldID(env, contextClass, "NOTIFICATION_SERVICE",
        "Ljava/lang/String;");
    jstring nsString = (*env)->GetStaticObjectField(env, contextClass, nsField);
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID getSystemService = (*env)->GetMethodID(env, activityClass, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject nm = (*env)->CallObjectMethod(env, g_activity, getSystemService, nsString);
    
    // Build notification using Notification.Builder
    jclass builderClass = (*env)->FindClass(env, "android/app/Notification$Builder");
    jmethodID builderInit = (*env)->GetMethodID(env, builderClass, "<init>",
        "(Landroid/content/Context;)V");
    jobject builder = (*env)->NewObject(env, builderClass, builderInit, g_activity);
    
    // Set title
    if (title) {
        jmethodID setTitle = (*env)->GetMethodID(env, builderClass, "setContentTitle",
            "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;");
        jstring jtitle = (*env)->NewStringUTF(env, title);
        (*env)->CallObjectMethod(env, builder, setTitle, jtitle);
    }
    
    // Set message
    if (message) {
        jmethodID setText = (*env)->GetMethodID(env, builderClass, "setContentText",
            "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;");
        jstring jmessage = (*env)->NewStringUTF(env, message);
        (*env)->CallObjectMethod(env, builder, setText, jmessage);
    }
    
    // Set small icon (required)
    jmethodID setIcon = (*env)->GetMethodID(env, builderClass, "setSmallIcon",
        "(I)Landroid/app/Notification$Builder;");
    (*env)->CallObjectMethod(env, builder, setIcon, 17301659); // android.R.drawable.ic_dialog_info
    
    // Build and notify
    jmethodID build = (*env)->GetMethodID(env, builderClass, "build", "()Landroid/app/Notification;");
    jobject notification = (*env)->CallObjectMethod(env, builder, build);
    
    jclass nmClass = (*env)->GetObjectClass(env, nm);
    jmethodID notify = (*env)->GetMethodID(env, nmClass, "notify", "(ILandroid/app/Notification;)V");
    (*env)->CallVoidMethod(env, nm, notify, 1, notification);
    
    return true;
}

#endif // HAL_NO_NOTIFICATION
