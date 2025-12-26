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

// Android bluetooth using BluetoothAdapter via JNI

#ifndef HAL_NO_BLUETOOTH
#include "hal/bluetooth.h"
#include <jni.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

static jobject get_bluetooth_adapter(JNIEnv *env) {
    jclass btAdapterClass = (*env)->FindClass(env, "android/bluetooth/BluetoothAdapter");
    jmethodID getDefault = (*env)->GetStaticMethodID(env, btAdapterClass, "getDefaultAdapter",
        "()Landroid/bluetooth/BluetoothAdapter;");
    return (*env)->CallStaticObjectMethod(env, btAdapterClass, getDefault);
}

bool hal_bluetooth_available(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    return get_bluetooth_adapter(env) != NULL;
}

bool hal_bluetooth_is_enabled(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject adapter = get_bluetooth_adapter(env);
    if (!adapter) return false;
    
    jclass adapterClass = (*env)->GetObjectClass(env, adapter);
    jmethodID isEnabled = (*env)->GetMethodID(env, adapterClass, "isEnabled", "()Z");
    
    return (*env)->CallBooleanMethod(env, adapter, isEnabled);
}

bool hal_bluetooth_enable(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject adapter = get_bluetooth_adapter(env);
    if (!adapter) return false;
    
    jclass adapterClass = (*env)->GetObjectClass(env, adapter);
    jmethodID enable = (*env)->GetMethodID(env, adapterClass, "enable", "()Z");
    
    return (*env)->CallBooleanMethod(env, adapter, enable);
}

bool hal_bluetooth_disable(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject adapter = get_bluetooth_adapter(env);
    if (!adapter) return false;
    
    jclass adapterClass = (*env)->GetObjectClass(env, adapter);
    jmethodID disable = (*env)->GetMethodID(env, adapterClass, "disable", "()Z");
    
    return (*env)->CallBooleanMethod(env, adapter, disable);
}

#endif // HAL_NO_BLUETOOTH
