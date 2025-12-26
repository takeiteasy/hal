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

// Android file chooser using Storage Access Framework via JNI
// Note: Due to async nature, this implementation uses a polling mechanism

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"
#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <string.h>

// Must be set by the application at startup
static JavaVM *g_jvm = NULL;
static jobject g_activity = NULL;

// Results from callbacks
static volatile bool g_result_ready = false;
static hal_file_chooser_result_t *g_pending_result = NULL;
static volatile int g_alert_result = -1;

void hal_file_chooser_android_init(JavaVM *jvm, jobject activity) {
    g_jvm = jvm;
    g_activity = activity;
}

// Called from Java when file picker returns
JNIEXPORT void JNICALL Java_hal_FileChooser_onFilesSelected(JNIEnv *env, jclass cls, jobjectArray paths) {
    (void)cls;
    
    if (paths == NULL) {
        g_pending_result = NULL;
    } else {
        int count = (*env)->GetArrayLength(env, paths);
        g_pending_result = malloc(sizeof(hal_file_chooser_result_t));
        g_pending_result->count = count;
        g_pending_result->paths = malloc(sizeof(char *) * (count + 1));
        
        for (int i = 0; i < count; i++) {
            jstring jpath = (jstring)(*env)->GetObjectArrayElement(env, paths, i);
            const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
            g_pending_result->paths[i] = strdup(path);
            (*env)->ReleaseStringUTFChars(env, jpath, path);
        }
        g_pending_result->paths[count] = NULL;
    }
    
    g_result_ready = true;
}

JNIEXPORT void JNICALL Java_hal_FileChooser_onAlertResult(JNIEnv *env, jclass cls, jint result) {
    (void)env;
    (void)cls;
    g_alert_result = result;
    g_result_ready = true;
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

bool hal_file_chooser_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    if (options == NULL || !hal_file_chooser_available())
        return NULL;
    
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return NULL;
    
    g_result_ready = false;
    g_pending_result = NULL;
    
    // Get HAL FileChooser helper class
    jclass helperClass = (*env)->FindClass(env, "hal/FileChooser");
    if (helperClass == NULL)
        return NULL;
    
    jmethodID showMethod = (*env)->GetStaticMethodID(env, helperClass, "show",
        "(Landroid/app/Activity;IZLjava/lang/String;)V");
    if (showMethod == NULL)
        return NULL;
    
    jstring jmimeType = NULL;
    if (options->filters && options->filters[0]) {
        // Convert first filter pattern to MIME type (simplified)
        const char *filter = options->filters[0];
        if (strstr(filter, ".jpg") || strstr(filter, ".jpeg") || strstr(filter, ".png")) {
            jmimeType = (*env)->NewStringUTF(env, "image/*");
        } else if (strstr(filter, ".pdf")) {
            jmimeType = (*env)->NewStringUTF(env, "application/pdf");
        } else if (strstr(filter, ".txt")) {
            jmimeType = (*env)->NewStringUTF(env, "text/plain");
        } else {
            jmimeType = (*env)->NewStringUTF(env, "*/*");
        }
    } else {
        jmimeType = (*env)->NewStringUTF(env, "*/*");
    }
    
    (*env)->CallStaticVoidMethod(env, helperClass, showMethod,
        g_activity, (jint)options->mode, (jboolean)options->allow_multiple, jmimeType);
    
    // Wait for result (with timeout)
    for (int i = 0; i < 30000 && !g_result_ready; i++) {
        usleep(1000); // 1ms
    }
    
    return g_pending_result;
}

void hal_file_chooser_result_free(hal_file_chooser_result_t *result) {
    if (result == NULL)
        return;
    
    if (result->paths != NULL) {
        for (int i = 0; i < result->count; i++) {
            free(result->paths[i]);
        }
        free(result->paths);
    }
    free(result);
}

int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count) {
    if (!hal_file_chooser_available())
        return -1;
    
    JNIEnv *env = get_jni_env();
    if (env == NULL)
        return -1;
    
    g_result_ready = false;
    g_alert_result = -1;
    
    jclass helperClass = (*env)->FindClass(env, "hal/FileChooser");
    if (helperClass == NULL)
        return -1;
    
    jmethodID alertMethod = (*env)->GetStaticMethodID(env, helperClass, "showAlert",
        "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V");
    if (alertMethod == NULL)
        return -1;
    
    jstring jtitle = title ? (*env)->NewStringUTF(env, title) : NULL;
    jstring jmessage = message ? (*env)->NewStringUTF(env, message) : NULL;
    
    jobjectArray jbuttons = NULL;
    if (buttons && button_count > 0) {
        jclass stringClass = (*env)->FindClass(env, "java/lang/String");
        jbuttons = (*env)->NewObjectArray(env, button_count, stringClass, NULL);
        for (int i = 0; i < button_count && buttons[i]; i++) {
            jstring jbtn = (*env)->NewStringUTF(env, buttons[i]);
            (*env)->SetObjectArrayElement(env, jbuttons, i, jbtn);
        }
    }
    
    (*env)->CallStaticVoidMethod(env, helperClass, alertMethod,
        g_activity, jtitle, jmessage, jbuttons);
    
    // Wait for result
    for (int i = 0; i < 60000 && !g_result_ready; i++) {
        usleep(1000);
    }
    
    return g_alert_result;
}

#endif // HAL_NO_FILE_CHOOSER
