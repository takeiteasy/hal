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

// Android clipboard using JNI to access ClipboardManager

#ifndef HAL_NO_CLIPBOARD
#include "hal/clipboard.h"
#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <string.h>

// These must be set by the application at startup
static JavaVM *g_jvm = NULL;
static jobject g_activity = NULL;

// Called by the application to initialize JNI references
void hal_clipboard_android_init(JavaVM *jvm, jobject activity) {
    g_jvm = jvm;
    g_activity = activity;
}

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL)
        return NULL;
    
    JNIEnv *env = NULL;
    int status = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    
    if (status == JNI_EDETACHED) {
        if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != 0)
            return NULL;
    } else if (status != JNI_OK) {
        return NULL;
    }
    
    return env;
}

static jobject get_clipboard_manager(JNIEnv *env) {
    if (g_activity == NULL)
        return NULL;
    
    jclass contextClass = (*env)->FindClass(env, "android/content/Context");
    if (contextClass == NULL)
        return NULL;
    
    jfieldID clipboardServiceField = (*env)->GetStaticFieldID(env, contextClass, 
        "CLIPBOARD_SERVICE", "Ljava/lang/String;");
    if (clipboardServiceField == NULL)
        return NULL;
    
    jstring clipboardService = (*env)->GetStaticObjectField(env, contextClass, clipboardServiceField);
    
    jmethodID getSystemService = (*env)->GetMethodID(env, contextClass, 
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    if (getSystemService == NULL)
        return NULL;
    
    return (*env)->CallObjectMethod(env, g_activity, getSystemService, clipboardService);
}

bool hal_clipboard_available(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return false;
    
    jobject clipboardManager = get_clipboard_manager(env);
    return clipboardManager != NULL;
}

bool hal_clipboard_has_text(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return false;
    
    jobject clipboardManager = get_clipboard_manager(env);
    if (clipboardManager == NULL)
        return false;
    
    jclass clipboardClass = (*env)->FindClass(env, "android/content/ClipboardManager");
    if (clipboardClass == NULL)
        return false;
    
    jmethodID hasText = (*env)->GetMethodID(env, clipboardClass, "hasText", "()Z");
    if (hasText == NULL)
        return false;
    
    return (*env)->CallBooleanMethod(env, clipboardManager, hasText);
}

char *hal_clipboard_get_text(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return NULL;
    
    jobject clipboardManager = get_clipboard_manager(env);
    if (clipboardManager == NULL)
        return NULL;
    
    jclass clipboardClass = (*env)->FindClass(env, "android/content/ClipboardManager");
    if (clipboardClass == NULL)
        return NULL;
    
    jmethodID getPrimaryClip = (*env)->GetMethodID(env, clipboardClass, 
        "getPrimaryClip", "()Landroid/content/ClipData;");
    if (getPrimaryClip == NULL)
        return NULL;
    
    jobject clipData = (*env)->CallObjectMethod(env, clipboardManager, getPrimaryClip);
    if (clipData == NULL)
        return NULL;
    
    jclass clipDataClass = (*env)->FindClass(env, "android/content/ClipData");
    jmethodID getItemAt = (*env)->GetMethodID(env, clipDataClass, 
        "getItemAt", "(I)Landroid/content/ClipData$Item;");
    
    jobject item = (*env)->CallObjectMethod(env, clipData, getItemAt, 0);
    if (item == NULL)
        return NULL;
    
    jclass itemClass = (*env)->FindClass(env, "android/content/ClipData$Item");
    jmethodID getText = (*env)->GetMethodID(env, itemClass, 
        "getText", "()Ljava/lang/CharSequence;");
    
    jobject charSeq = (*env)->CallObjectMethod(env, item, getText);
    if (charSeq == NULL)
        return NULL;
    
    jclass charSeqClass = (*env)->FindClass(env, "java/lang/CharSequence");
    jmethodID toString = (*env)->GetMethodID(env, charSeqClass, 
        "toString", "()Ljava/lang/String;");
    
    jstring jstr = (jstring)(*env)->CallObjectMethod(env, charSeq, toString);
    if (jstr == NULL)
        return NULL;
    
    const char *utf8 = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (utf8 == NULL)
        return NULL;
    
    char *result = strdup(utf8);
    (*env)->ReleaseStringUTFChars(env, jstr, utf8);
    
    return result;
}

bool hal_clipboard_set_text(const char *text) {
    if (text == NULL)
        return false;
    
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return false;
    
    jobject clipboardManager = get_clipboard_manager(env);
    if (clipboardManager == NULL)
        return false;
    
    jclass clipDataClass = (*env)->FindClass(env, "android/content/ClipData");
    if (clipDataClass == NULL)
        return false;
    
    jmethodID newPlainText = (*env)->GetStaticMethodID(env, clipDataClass, "newPlainText",
        "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;");
    if (newPlainText == NULL)
        return false;
    
    jstring label = (*env)->NewStringUTF(env, "HAL Clipboard");
    jstring jtext = (*env)->NewStringUTF(env, text);
    
    jobject clipData = (*env)->CallStaticObjectMethod(env, clipDataClass, 
        newPlainText, label, jtext);
    if (clipData == NULL)
        return false;
    
    jclass clipboardClass = (*env)->FindClass(env, "android/content/ClipboardManager");
    jmethodID setPrimaryClip = (*env)->GetMethodID(env, clipboardClass, 
        "setPrimaryClip", "(Landroid/content/ClipData;)V");
    if (setPrimaryClip == NULL)
        return false;
    
    (*env)->CallVoidMethod(env, clipboardManager, setPrimaryClip, clipData);
    
    return true;
}

void hal_clipboard_clear(void) {
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return;
    
    jobject clipboardManager = get_clipboard_manager(env);
    if (clipboardManager == NULL)
        return;
    
    jclass clipboardClass = (*env)->FindClass(env, "android/content/ClipboardManager");
    if (clipboardClass == NULL)
        return;
    
    // clearPrimaryClip requires API 28+, fallback to setting empty text
    jmethodID clearPrimaryClip = (*env)->GetMethodID(env, clipboardClass, 
        "clearPrimaryClip", "()V");
    
    if (clearPrimaryClip != NULL) {
        (*env)->CallVoidMethod(env, clipboardManager, clearPrimaryClip);
    } else {
        // Fallback: set empty clip data
        hal_clipboard_set_text("");
    }
}

#endif // HAL_NO_CLIPBOARD
