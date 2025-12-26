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

// Android camera using Intent

#ifndef HAL_NO_CAMERA
#include "hal/camera.h"
#include <jni.h>
#include <stdlib.h>
#include <string.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static hal_camera_cb g_camera_cb = NULL;
static void *g_camera_ctx = NULL;

// Called from Java when capture completes
JNIEXPORT void JNICALL Java_hal_Camera_onCaptureComplete(JNIEnv *env, jclass cls, jstring path) {
    (void)cls;
    if (g_camera_cb) {
        if (path) {
            const char *str = (*env)->GetStringUTFChars(env, path, NULL);
            g_camera_cb(str, g_camera_ctx);
            (*env)->ReleaseStringUTFChars(env, path, str);
        } else {
            g_camera_cb(NULL, g_camera_ctx);
        }
    }
    g_camera_cb = NULL;
    g_camera_ctx = NULL;
}

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

bool hal_camera_available(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    // Check if device has camera
    jclass pmClass = (*env)->FindClass(env, "android/content/pm/PackageManager");
    jfieldID cameraFeature = (*env)->GetStaticFieldID(env, pmClass, "FEATURE_CAMERA_ANY",
        "Ljava/lang/String;");
    jstring feature = (*env)->GetStaticObjectField(env, pmClass, cameraFeature);
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID getPM = (*env)->GetMethodID(env, activityClass, "getPackageManager",
        "()Landroid/content/pm/PackageManager;");
    jobject pm = (*env)->CallObjectMethod(env, g_activity, getPM);
    
    jmethodID hasFeature = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, pm),
        "hasSystemFeature", "(Ljava/lang/String;)Z");
    
    return (*env)->CallBooleanMethod(env, pm, hasFeature, feature);
}

static bool start_capture(const char *filename, hal_camera_cb on_complete, void *ctx, 
                          const char *action) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return false;
    
    g_camera_cb = on_complete;
    g_camera_ctx = ctx;
    
    // Create intent
    jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
    jstring jaction = (*env)->NewStringUTF(env, action);
    jmethodID intentInit = (*env)->GetMethodID(env, intentClass, "<init>",
        "(Ljava/lang/String;)V");
    jobject intent = (*env)->NewObject(env, intentClass, intentInit, jaction);
    
    // Start activity for result
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID startActivity = (*env)->GetMethodID(env, activityClass, 
        "startActivityForResult", "(Landroid/content/Intent;I)V");
    (*env)->CallVoidMethod(env, g_activity, startActivity, intent, 1001);
    
    (void)filename; // Android handles the filename internally
    return true;
}

bool hal_camera_take_picture(const char *filename, hal_camera_cb on_complete, void *ctx) {
    return start_capture(filename, on_complete, ctx, "android.media.action.IMAGE_CAPTURE");
}

bool hal_camera_take_video(const char *filename, hal_camera_cb on_complete, void *ctx) {
    return start_capture(filename, on_complete, ctx, "android.media.action.VIDEO_CAPTURE");
}

#endif // HAL_NO_CAMERA
