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

// Android flash using Camera2 API (API 21+) or Camera API

#ifndef HAL_NO_FLASH
#include "hal/flash.h"
#include <jni.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static bool flash_state = false;

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

bool hal_flash_available(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return false;
    
    // Check PackageManager.FEATURE_CAMERA_FLASH
    jclass context_class = (*env)->FindClass(env, "android/content/Context");
    jmethodID get_pm = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, g_activity),
        "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject pm = (*env)->CallObjectMethod(env, g_activity, get_pm);
    if (!pm)
        return false;
    
    jclass pm_class = (*env)->FindClass(env, "android/content/pm/PackageManager");
    jfieldID flash_field = (*env)->GetStaticFieldID(env, pm_class, "FEATURE_CAMERA_FLASH", "Ljava/lang/String;");
    jstring flash_feature = (jstring)(*env)->GetStaticObjectField(env, pm_class, flash_field);
    
    jmethodID has_feature = (*env)->GetMethodID(env, pm_class, "hasSystemFeature", "(Ljava/lang/String;)Z");
    jboolean result = (*env)->CallBooleanMethod(env, pm, has_feature, flash_feature);
    
    (*env)->DeleteLocalRef(env, pm);
    return result;
}

void hal_flash_on(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return;
    
    // Get CameraManager
    jclass context_class = (*env)->FindClass(env, "android/content/Context");
    jfieldID camera_field = (*env)->GetStaticFieldID(env, context_class, "CAMERA_SERVICE", "Ljava/lang/String;");
    jstring camera_service = (jstring)(*env)->GetStaticObjectField(env, context_class, camera_field);
    
    jmethodID get_service = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, g_activity),
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject camera_manager = (*env)->CallObjectMethod(env, g_activity, get_service, camera_service);
    if (!camera_manager)
        return;
    
    // Get camera ID list
    jclass cm_class = (*env)->GetObjectClass(env, camera_manager);
    jmethodID get_ids = (*env)->GetMethodID(env, cm_class, "getCameraIdList", "()[Ljava/lang/String;");
    jobjectArray ids = (jobjectArray)(*env)->CallObjectMethod(env, camera_manager, get_ids);
    if (!ids || (*env)->GetArrayLength(env, ids) == 0)
        return;
    
    // Use first camera (usually back camera with flash)
    jstring camera_id = (jstring)(*env)->GetObjectArrayElement(env, ids, 0);
    
    // Set torch mode
    jmethodID set_torch = (*env)->GetMethodID(env, cm_class, "setTorchMode", "(Ljava/lang/String;Z)V");
    (*env)->CallVoidMethod(env, camera_manager, set_torch, camera_id, JNI_TRUE);
    
    flash_state = true;
    (*env)->DeleteLocalRef(env, camera_manager);
}

void hal_flash_off(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return;
    
    jclass context_class = (*env)->FindClass(env, "android/content/Context");
    jfieldID camera_field = (*env)->GetStaticFieldID(env, context_class, "CAMERA_SERVICE", "Ljava/lang/String;");
    jstring camera_service = (jstring)(*env)->GetStaticObjectField(env, context_class, camera_field);
    
    jmethodID get_service = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, g_activity),
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject camera_manager = (*env)->CallObjectMethod(env, g_activity, get_service, camera_service);
    if (!camera_manager)
        return;
    
    jclass cm_class = (*env)->GetObjectClass(env, camera_manager);
    jmethodID get_ids = (*env)->GetMethodID(env, cm_class, "getCameraIdList", "()[Ljava/lang/String;");
    jobjectArray ids = (jobjectArray)(*env)->CallObjectMethod(env, camera_manager, get_ids);
    if (!ids || (*env)->GetArrayLength(env, ids) == 0)
        return;
    
    jstring camera_id = (jstring)(*env)->GetObjectArrayElement(env, ids, 0);
    jmethodID set_torch = (*env)->GetMethodID(env, cm_class, "setTorchMode", "(Ljava/lang/String;Z)V");
    (*env)->CallVoidMethod(env, camera_manager, set_torch, camera_id, JNI_FALSE);
    
    flash_state = false;
    (*env)->DeleteLocalRef(env, camera_manager);
}

bool hal_flash_toggle(void) {
    if (flash_state) {
        hal_flash_off();
    } else {
        hal_flash_on();
    }
    return flash_state;
}
#endif // HAL_NO_FLASH
