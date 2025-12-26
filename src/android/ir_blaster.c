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

// Android IR Blaster using ConsumerIrManager

#ifndef HAL_NO_IR_BLASTER
#include "hal/ir_blaster.h"
#include <jni.h>
#include <stdlib.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

static jobject get_ir_manager(JNIEnv *env) {
    jclass contextClass = (*env)->FindClass(env, "android/content/Context");
    jfieldID irField = (*env)->GetStaticFieldID(env, contextClass, "CONSUMER_IR_SERVICE",
        "Ljava/lang/String;");
    jstring irService = (*env)->GetStaticObjectField(env, contextClass, irField);
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID getSystemService = (*env)->GetMethodID(env, activityClass, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    
    return (*env)->CallObjectMethod(env, g_activity, getSystemService, irService);
}

bool hal_ir_available(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jobject irManager = get_ir_manager(env);
    if (irManager == NULL) return false;
    
    jclass irClass = (*env)->GetObjectClass(env, irManager);
    jmethodID hasIrEmitter = (*env)->GetMethodID(env, irClass, "hasIrEmitter", "()Z");
    
    return (*env)->CallBooleanMethod(env, irManager, hasIrEmitter);
}

bool hal_ir_transmit(int frequency, const int *pattern, int pattern_len) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return false;
    
    jobject irManager = get_ir_manager(env);
    if (irManager == NULL) return false;
    
    jclass irClass = (*env)->GetObjectClass(env, irManager);
    jmethodID transmit = (*env)->GetMethodID(env, irClass, "transmit", "(I[I)V");
    
    jintArray jpattern = (*env)->NewIntArray(env, pattern_len);
    (*env)->SetIntArrayRegion(env, jpattern, 0, pattern_len, pattern);
    
    (*env)->CallVoidMethod(env, irManager, transmit, frequency, jpattern);
    
    return true;
}

int hal_ir_get_frequencies(int *min_freq, int *max_freq, int max_ranges) {
    JNIEnv *env = get_jni_env();
    if (env == NULL) return 0;
    
    jobject irManager = get_ir_manager(env);
    if (irManager == NULL) return 0;
    
    jclass irClass = (*env)->GetObjectClass(env, irManager);
    jmethodID getFreqs = (*env)->GetMethodID(env, irClass, "getCarrierFrequencies",
        "()[Landroid/hardware/ConsumerIrManager$CarrierFrequencyRange;");
    
    jobjectArray ranges = (*env)->CallObjectMethod(env, irManager, getFreqs);
    if (ranges == NULL) return 0;
    
    int count = (*env)->GetArrayLength(env, ranges);
    if (count > max_ranges) count = max_ranges;
    
    jclass rangeClass = (*env)->FindClass(env, "android/hardware/ConsumerIrManager$CarrierFrequencyRange");
    jmethodID getMin = (*env)->GetMethodID(env, rangeClass, "getMinFrequency", "()I");
    jmethodID getMax = (*env)->GetMethodID(env, rangeClass, "getMaxFrequency", "()I");
    
    for (int i = 0; i < count; i++) {
        jobject range = (*env)->GetObjectArrayElement(env, ranges, i);
        if (min_freq) min_freq[i] = (*env)->CallIntMethod(env, range, getMin);
        if (max_freq) max_freq[i] = (*env)->CallIntMethod(env, range, getMax);
    }
    
    return count;
}

#endif // HAL_NO_IR_BLASTER
