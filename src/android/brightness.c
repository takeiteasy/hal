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

#ifndef HAL_NO_BRIGHTNESS
#include "hal/brightness.h"
#include <jni.h>
#include <stdbool.h>

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

bool hal_brightness_available(void) {
    JNIEnv *env = get_jni_env();
    return env != NULL && g_activity != NULL;
}

float hal_brightness_get(void) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return -1.0f;
    
    // Get window
    jclass activity_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_window = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
    jobject window = (*env)->CallObjectMethod(env, g_activity, get_window);
    if (!window)
        return -1.0f;
    
    // Get window attributes
    jclass window_class = (*env)->GetObjectClass(env, window);
    jmethodID get_attrs = (*env)->GetMethodID(env, window_class, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
    jobject attrs = (*env)->CallObjectMethod(env, window, get_attrs);
    if (!attrs)
        return -1.0f;
    
    // Get screenBrightness field
    jclass params_class = (*env)->GetObjectClass(env, attrs);
    jfieldID brightness_field = (*env)->GetFieldID(env, params_class, "screenBrightness", "F");
    float brightness = (*env)->GetFloatField(env, attrs, brightness_field);
    
    (*env)->DeleteLocalRef(env, attrs);
    (*env)->DeleteLocalRef(env, window);
    
    // -1 means system default, return 0.5 as approximation
    if (brightness < 0)
        return 0.5f;
    
    return brightness;
}

bool hal_brightness_set(float level) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity)
        return false;
    
    // Clamp level to valid range
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    // Get window
    jclass activity_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_window = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
    jobject window = (*env)->CallObjectMethod(env, g_activity, get_window);
    if (!window)
        return false;
    
    // Get window attributes
    jclass window_class = (*env)->GetObjectClass(env, window);
    jmethodID get_attrs = (*env)->GetMethodID(env, window_class, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
    jobject attrs = (*env)->CallObjectMethod(env, window, get_attrs);
    if (!attrs) {
        (*env)->DeleteLocalRef(env, window);
        return false;
    }
    
    // Set screenBrightness field
    jclass params_class = (*env)->GetObjectClass(env, attrs);
    jfieldID brightness_field = (*env)->GetFieldID(env, params_class, "screenBrightness", "F");
    (*env)->SetFloatField(env, attrs, brightness_field, level);
    
    // Apply attributes
    jmethodID set_attrs = (*env)->GetMethodID(env, window_class, "setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V");
    (*env)->CallVoidMethod(env, window, set_attrs, attrs);
    
    (*env)->DeleteLocalRef(env, attrs);
    (*env)->DeleteLocalRef(env, window);
    
    return true;
}

#endif // HAL_NO_BRIGHTNESS
