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

// Android device name using android.os.Build.MODEL via JNI

#ifndef HAL_NO_DEVICE_NAME
#include "hal/device_name.h"
#include <jni.h>
#include <string.h>

// External reference to the JNI environment
extern JavaVM *g_jvm;

static char device_name_buffer[256] = {0};

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

bool hal_device_name_available(void) {
    return g_jvm != NULL;
}

const char* hal_device_name_get(void) {
    JNIEnv *env = get_jni_env();
    if (!env)
        return NULL;
    
    // Get android.os.Build class
    jclass build_class = (*env)->FindClass(env, "android/os/Build");
    if (!build_class)
        return NULL;
    
    // Get MODEL static field
    jfieldID model_field = (*env)->GetStaticFieldID(env, build_class, "MODEL", "Ljava/lang/String;");
    if (!model_field)
        return NULL;
    
    jstring model = (jstring)(*env)->GetStaticObjectField(env, build_class, model_field);
    if (!model)
        return NULL;
    
    const char *model_str = (*env)->GetStringUTFChars(env, model, NULL);
    if (model_str) {
        strncpy(device_name_buffer, model_str, sizeof(device_name_buffer) - 1);
        device_name_buffer[sizeof(device_name_buffer) - 1] = '\0';
        (*env)->ReleaseStringUTFChars(env, model, model_str);
    }
    
    (*env)->DeleteLocalRef(env, model);
    
    return device_name_buffer;
}
#endif // HAL_NO_DEVICE_NAME
