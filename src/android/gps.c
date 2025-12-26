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

// Android GPS using LocationManager via JNI

#ifndef HAL_NO_GPS
#include "hal/gps.h"
#include <jni.h>
#include <stdlib.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static hal_gps_location_cb g_location_cb = NULL;
static hal_gps_status_cb g_status_cb = NULL;
static void *g_ctx = NULL;

// Called from Java location listener
JNIEXPORT void JNICALL Java_hal_GPS_onLocation(JNIEnv *env, jclass cls,
        jdouble lat, jdouble lon, jdouble alt, jfloat speed, jfloat bearing) {
    (void)env; (void)cls;
    if (g_location_cb) {
        g_location_cb(lat, lon, alt, speed, bearing, g_ctx);
    }
}

JNIEXPORT void JNICALL Java_hal_GPS_onStatus(JNIEnv *env, jclass cls, jstring status) {
    (void)cls;
    if (g_status_cb && status) {
        const char *str = (*env)->GetStringUTFChars(env, status, NULL);
        g_status_cb(str, g_ctx);
        (*env)->ReleaseStringUTFChars(env, status, str);
    }
}

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

bool hal_gps_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

void hal_gps_configure(hal_gps_location_cb on_location, hal_gps_status_cb on_status, void *ctx) {
    g_location_cb = on_location;
    g_status_cb = on_status;
    g_ctx = ctx;
}

bool hal_gps_start(int min_time_ms, float min_distance_m) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jclass gpsClass = (*env)->FindClass(env, "hal/GPS");
    if (gpsClass == NULL) return false;
    
    jmethodID startMethod = (*env)->GetStaticMethodID(env, gpsClass, "start",
        "(Landroid/app/Activity;IF)Z");
    if (startMethod == NULL) return false;
    
    return (*env)->CallStaticBooleanMethod(env, gpsClass, startMethod,
        g_activity, min_time_ms, min_distance_m);
}

void hal_gps_stop(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return;
    
    jclass gpsClass = (*env)->FindClass(env, "hal/GPS");
    if (gpsClass == NULL) return;
    
    jmethodID stopMethod = (*env)->GetStaticMethodID(env, gpsClass, "stop", "()V");
    if (stopMethod) {
        (*env)->CallStaticVoidMethod(env, gpsClass, stopMethod);
    }
}

#endif // HAL_NO_GPS
