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

#ifndef HAL_NO_BATTERY
#include "hal/battery.h"
#include <jni.h>
#include <android/native_activity.h>
#include <stdbool.h>

// Android battery status constants (from BatteryManager)
#define BATTERY_STATUS_UNKNOWN 1
#define BATTERY_STATUS_CHARGING 2
#define BATTERY_STATUS_DISCHARGING 3
#define BATTERY_STATUS_NOT_CHARGING 4
#define BATTERY_STATUS_FULL 5

// External reference to the JNI environment - must be provided by the application
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

static jobject get_battery_intent(JNIEnv *env) {
    if (!env || !g_activity)
        return NULL;
    
    // Get Context class
    jclass context_class = (*env)->FindClass(env, "android/content/Context");
    if (!context_class)
        return NULL;
    
    // Get registerReceiver method
    jmethodID register_receiver = (*env)->GetMethodID(env, context_class, 
        "registerReceiver", 
        "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;");
    if (!register_receiver)
        return NULL;
    
    // Create IntentFilter for ACTION_BATTERY_CHANGED
    jclass intent_filter_class = (*env)->FindClass(env, "android/content/IntentFilter");
    jmethodID intent_filter_init = (*env)->GetMethodID(env, intent_filter_class, "<init>", "(Ljava/lang/String;)V");
    jstring action = (*env)->NewStringUTF(env, "android.intent.action.BATTERY_CHANGED");
    jobject intent_filter = (*env)->NewObject(env, intent_filter_class, intent_filter_init, action);
    
    // Register receiver (null receiver returns sticky intent)
    jobject battery_intent = (*env)->CallObjectMethod(env, g_activity, register_receiver, NULL, intent_filter);
    
    (*env)->DeleteLocalRef(env, action);
    (*env)->DeleteLocalRef(env, intent_filter);
    
    return battery_intent;
}

static int get_battery_int_extra(JNIEnv *env, jobject intent, const char *name, int default_value) {
    if (!env || !intent)
        return default_value;
    
    jclass intent_class = (*env)->GetObjectClass(env, intent);
    jmethodID get_int_extra = (*env)->GetMethodID(env, intent_class, "getIntExtra", "(Ljava/lang/String;I)I");
    jstring key = (*env)->NewStringUTF(env, name);
    int result = (*env)->CallIntMethod(env, intent, get_int_extra, key, default_value);
    (*env)->DeleteLocalRef(env, key);
    return result;
}

bool hal_battery_available(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return false;
    
    jobject intent = get_battery_intent(env);
    bool available = (intent != NULL);
    if (intent)
        (*env)->DeleteLocalRef(env, intent);
    
    return available;
}

int hal_battery_level(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return -1;
    
    jobject intent = get_battery_intent(env);
    if (!intent)
        return -1;
    
    int level = get_battery_int_extra(env, intent, "level", -1);
    int scale = get_battery_int_extra(env, intent, "scale", 100);
    
    (*env)->DeleteLocalRef(env, intent);
    
    if (level < 0 || scale <= 0)
        return -1;
    
    return (level * 100) / scale;
}

hal_battery_status_t hal_battery_status(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return HAL_BATTERY_STATUS_UNKNOWN;
    
    jobject intent = get_battery_intent(env);
    if (!intent)
        return HAL_BATTERY_STATUS_UNKNOWN;
    
    int status = get_battery_int_extra(env, intent, "status", BATTERY_STATUS_UNKNOWN);
    (*env)->DeleteLocalRef(env, intent);
    
    switch (status) {
        case BATTERY_STATUS_CHARGING:
            return HAL_BATTERY_STATUS_CHARGING;
        case BATTERY_STATUS_DISCHARGING:
            return HAL_BATTERY_STATUS_DISCHARGING;
        case BATTERY_STATUS_FULL:
            return HAL_BATTERY_STATUS_FULL;
        case BATTERY_STATUS_NOT_CHARGING:
            return HAL_BATTERY_STATUS_NOT_CHARGING;
        default:
            return HAL_BATTERY_STATUS_UNKNOWN;
    }
}

bool hal_battery_is_charging(void) {
    hal_battery_status_t status = hal_battery_status();
    return status == HAL_BATTERY_STATUS_CHARGING;
}

bool hal_battery_is_plugged(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return false;
    
    jobject intent = get_battery_intent(env);
    if (!intent)
        return false;
    
    int plugged = get_battery_int_extra(env, intent, "plugged", 0);
    (*env)->DeleteLocalRef(env, intent);
    
    return plugged != 0;
}

#endif // HAL_NO_BATTERY
