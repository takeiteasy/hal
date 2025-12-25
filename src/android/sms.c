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

// Android SMS using SmsManager

#ifndef HAL_NO_SMS
#include "hal/sms.h"
#include <jni.h>
#include <string.h>

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

bool hal_sms_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_sms_send(const char *recipient, const char *message) {
    JNIEnv *env = get_jni_env();
    if (!env || !recipient || !message)
        return false;
    
    // Get SmsManager.getDefault()
    jclass sms_class = (*env)->FindClass(env, "android/telephony/SmsManager");
    if (!sms_class)
        return false;
    
    jmethodID get_default = (*env)->GetStaticMethodID(env, sms_class, 
        "getDefault", "()Landroid/telephony/SmsManager;");
    jobject sms_manager = (*env)->CallStaticObjectMethod(env, sms_class, get_default);
    if (!sms_manager)
        return false;
    
    // Create Java strings
    jstring j_recipient = (*env)->NewStringUTF(env, recipient);
    jstring j_message = (*env)->NewStringUTF(env, message);
    
    // Call sendTextMessage(recipient, null, message, null, null)
    jmethodID send_text = (*env)->GetMethodID(env, sms_class, "sendTextMessage",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/app/PendingIntent;Landroid/app/PendingIntent;)V");
    
    (*env)->CallVoidMethod(env, sms_manager, send_text, j_recipient, NULL, j_message, NULL, NULL);
    
    (*env)->DeleteLocalRef(env, j_recipient);
    (*env)->DeleteLocalRef(env, j_message);
    (*env)->DeleteLocalRef(env, sms_manager);
    
    return true;
}
#endif
