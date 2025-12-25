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

// Android storagepath using JNI to get app directories

#ifndef HAL_NO_STORAGEPATH
#include "hal/storagepath.h"
#include <jni.h>
#include <string.h>
#include <stdlib.h>

extern JavaVM *g_jvm;
extern jobject g_activity;

static char home_buf[1024];
static char documents_buf[1024];
static char downloads_buf[1024];
static char cache_buf[1024];
static char temp_buf[1024];
static char external_buf[1024];

static JNIEnv* get_jni_env(void) {
    JNIEnv *env = NULL;
    if (g_jvm) {
        (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
        if (!env)
            (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
    }
    return env;
}

static const char* get_files_dir(char *buf) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return NULL;
    
    jclass context_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_files = (*env)->GetMethodID(env, context_class, "getFilesDir", "()Ljava/io/File;");
    jobject files_dir = (*env)->CallObjectMethod(env, g_activity, get_files);
    if (!files_dir) return NULL;
    
    jclass file_class = (*env)->GetObjectClass(env, files_dir);
    jmethodID get_path = (*env)->GetMethodID(env, file_class, "getAbsolutePath", "()Ljava/lang/String;");
    jstring path_str = (*env)->CallObjectMethod(env, files_dir, get_path);
    
    const char *path = (*env)->GetStringUTFChars(env, path_str, NULL);
    strncpy(buf, path, 1023);
    buf[1023] = '\0';
    (*env)->ReleaseStringUTFChars(env, path_str, path);
    
    return buf;
}

static const char* get_cache_dir(char *buf) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return NULL;
    
    jclass context_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_cache = (*env)->GetMethodID(env, context_class, "getCacheDir", "()Ljava/io/File;");
    jobject cache_dir = (*env)->CallObjectMethod(env, g_activity, get_cache);
    if (!cache_dir) return NULL;
    
    jclass file_class = (*env)->GetObjectClass(env, cache_dir);
    jmethodID get_path = (*env)->GetMethodID(env, file_class, "getAbsolutePath", "()Ljava/lang/String;");
    jstring path_str = (*env)->CallObjectMethod(env, cache_dir, get_path);
    
    const char *path = (*env)->GetStringUTFChars(env, path_str, NULL);
    strncpy(buf, path, 1023);
    buf[1023] = '\0';
    (*env)->ReleaseStringUTFChars(env, path_str, path);
    
    return buf;
}

static const char* get_external_files_dir(char *buf) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return NULL;
    
    jclass context_class = (*env)->GetObjectClass(env, g_activity);
    jmethodID get_ext = (*env)->GetMethodID(env, context_class, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
    jobject ext_dir = (*env)->CallObjectMethod(env, g_activity, get_ext, NULL);
    if (!ext_dir) return NULL;
    
    jclass file_class = (*env)->GetObjectClass(env, ext_dir);
    jmethodID get_path = (*env)->GetMethodID(env, file_class, "getAbsolutePath", "()Ljava/lang/String;");
    jstring path_str = (*env)->CallObjectMethod(env, ext_dir, get_path);
    
    const char *path = (*env)->GetStringUTFChars(env, path_str, NULL);
    strncpy(buf, path, 1023);
    buf[1023] = '\0';
    (*env)->ReleaseStringUTFChars(env, path_str, path);
    
    return buf;
}

bool hal_storagepath_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

const char* hal_path_home(void) {
    return get_files_dir(home_buf);
}

const char* hal_path_documents(void) {
    return get_external_files_dir(documents_buf);
}

const char* hal_path_downloads(void) {
    // Android doesn't have a per-app downloads folder
    return get_external_files_dir(downloads_buf);
}

const char* hal_path_desktop(void) { return NULL; }
const char* hal_path_pictures(void) { return NULL; }
const char* hal_path_music(void) { return NULL; }
const char* hal_path_videos(void) { return NULL; }

const char* hal_path_appdata(void) {
    return get_files_dir(home_buf);
}

const char* hal_path_cache(void) {
    return get_cache_dir(cache_buf);
}

const char* hal_path_temp(void) {
    return get_cache_dir(temp_buf);
}
#endif
