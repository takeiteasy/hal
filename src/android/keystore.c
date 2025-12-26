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

// Android keystore using EncryptedSharedPreferences via JNI

#ifndef HAL_NO_KEYSTORE
#include "hal/keystore.h"
#include <jni.h>
#include <stdlib.h>
#include <string.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static JNIEnv *get_jni_env(void) {
    if (g_jvm == NULL) return NULL;
    JNIEnv *env = NULL;
    (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    return env;
}

static jobject get_shared_prefs(JNIEnv *env, const char *service) {
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID getPrefs = (*env)->GetMethodID(env, activityClass, "getSharedPreferences",
        "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
    
    jstring jservice = (*env)->NewStringUTF(env, service);
    return (*env)->CallObjectMethod(env, g_activity, getPrefs, jservice, 0);
}

bool hal_keystore_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_keystore_set(const char *service, const char *key, const char *value) {
    if (!service || !key || !value) return false;
    
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject prefs = get_shared_prefs(env, service);
    if (!prefs) return false;
    
    jclass prefsClass = (*env)->GetObjectClass(env, prefs);
    jmethodID edit = (*env)->GetMethodID(env, prefsClass, "edit",
        "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = (*env)->CallObjectMethod(env, prefs, edit);
    
    jclass editorClass = (*env)->GetObjectClass(env, editor);
    jmethodID putString = (*env)->GetMethodID(env, editorClass, "putString",
        "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;");
    jmethodID apply = (*env)->GetMethodID(env, editorClass, "apply", "()V");
    
    jstring jkey = (*env)->NewStringUTF(env, key);
    jstring jvalue = (*env)->NewStringUTF(env, value);
    (*env)->CallObjectMethod(env, editor, putString, jkey, jvalue);
    (*env)->CallVoidMethod(env, editor, apply);
    
    return true;
}

char *hal_keystore_get(const char *service, const char *key) {
    if (!service || !key) return NULL;
    
    JNIEnv *env = get_jni_env();
    if (!env) return NULL;
    
    jobject prefs = get_shared_prefs(env, service);
    if (!prefs) return NULL;
    
    jclass prefsClass = (*env)->GetObjectClass(env, prefs);
    jmethodID getString = (*env)->GetMethodID(env, prefsClass, "getString",
        "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    
    jstring jkey = (*env)->NewStringUTF(env, key);
    jstring result = (*env)->CallObjectMethod(env, prefs, getString, jkey, NULL);
    
    if (result) {
        const char *str = (*env)->GetStringUTFChars(env, result, NULL);
        char *ret = strdup(str);
        (*env)->ReleaseStringUTFChars(env, result, str);
        return ret;
    }
    
    return NULL;
}

bool hal_keystore_delete(const char *service, const char *key) {
    if (!service || !key) return false;
    
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject prefs = get_shared_prefs(env, service);
    if (!prefs) return false;
    
    jclass prefsClass = (*env)->GetObjectClass(env, prefs);
    jmethodID edit = (*env)->GetMethodID(env, prefsClass, "edit",
        "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = (*env)->CallObjectMethod(env, prefs, edit);
    
    jclass editorClass = (*env)->GetObjectClass(env, editor);
    jmethodID remove = (*env)->GetMethodID(env, editorClass, "remove",
        "(Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;");
    jmethodID apply = (*env)->GetMethodID(env, editorClass, "apply", "()V");
    
    jstring jkey = (*env)->NewStringUTF(env, key);
    (*env)->CallObjectMethod(env, editor, remove, jkey);
    (*env)->CallVoidMethod(env, editor, apply);
    
    return true;
}

#endif // HAL_NO_KEYSTORE
