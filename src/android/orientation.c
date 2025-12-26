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

// Android orientation using Activity.setRequestedOrientation

#ifndef HAL_NO_ORIENTATION
#include "hal/orientation.h"
#include <jni.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

static bool set_orientation(int orientation) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID setOrientation = (*env)->GetMethodID(env, activityClass, 
        "setRequestedOrientation", "(I)V");
    
    (*env)->CallVoidMethod(env, g_activity, setOrientation, orientation);
    return true;
}

bool hal_orientation_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_orientation_set_landscape(bool reverse) {
    // SCREEN_ORIENTATION_LANDSCAPE = 0, SCREEN_ORIENTATION_REVERSE_LANDSCAPE = 8
    return set_orientation(reverse ? 8 : 0);
}

bool hal_orientation_set_portrait(bool reverse) {
    // SCREEN_ORIENTATION_PORTRAIT = 1, SCREEN_ORIENTATION_REVERSE_PORTRAIT = 9
    return set_orientation(reverse ? 9 : 1);
}

bool hal_orientation_set_sensor(void) {
    // SCREEN_ORIENTATION_SENSOR = 4
    return set_orientation(4);
}

bool hal_orientation_lock(void) {
    // SCREEN_ORIENTATION_LOCKED = 14
    return set_orientation(14);
}

bool hal_orientation_unlock(void) {
    // SCREEN_ORIENTATION_UNSPECIFIED = -1
    return set_orientation(-1);
}

#endif // HAL_NO_ORIENTATION
