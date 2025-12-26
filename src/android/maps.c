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

// Android maps using geo: URI Intent

#ifndef HAL_NO_MAPS
#include "hal/maps.h"
#include <jni.h>
#include <stdio.h>
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

static bool open_geo_uri(const char *uri) {
    JNIEnv *env = get_jni_env();
    if (!env || !g_activity) return false;
    
    jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
    jmethodID parse = (*env)->GetStaticMethodID(env, uriClass, "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;");
    
    jstring juri = (*env)->NewStringUTF(env, uri);
    jobject uriObj = (*env)->CallStaticObjectMethod(env, uriClass, parse, juri);
    
    jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
    jfieldID actionView = (*env)->GetStaticFieldID(env, intentClass, "ACTION_VIEW",
        "Ljava/lang/String;");
    jstring action = (*env)->GetStaticObjectField(env, intentClass, actionView);
    
    jmethodID intentInit = (*env)->GetMethodID(env, intentClass, "<init>",
        "(Ljava/lang/String;Landroid/net/Uri;)V");
    jobject intent = (*env)->NewObject(env, intentClass, intentInit, action, uriObj);
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity",
        "(Landroid/content/Intent;)V");
    (*env)->CallVoidMethod(env, g_activity, startActivity, intent);
    
    return true;
}

bool hal_maps_available(void) {
    return g_jvm != NULL && g_activity != NULL;
}

bool hal_maps_open_address(const char *address) {
    if (!address) return false;
    char uri[1024];
    snprintf(uri, sizeof(uri), "geo:0,0?q=%s", address);
    return open_geo_uri(uri);
}

bool hal_maps_open_coordinates(double lat, double lon, const char *label) {
    char uri[512];
    if (label) {
        snprintf(uri, sizeof(uri), "geo:%f,%f?q=%f,%f(%s)", lat, lon, lat, lon, label);
    } else {
        snprintf(uri, sizeof(uri), "geo:%f,%f", lat, lon);
    }
    return open_geo_uri(uri);
}

bool hal_maps_search(const char *query, double lat, double lon) {
    if (!query) return false;
    char uri[1024];
    if (lat != 0 || lon != 0) {
        snprintf(uri, sizeof(uri), "geo:%f,%f?q=%s", lat, lon, query);
    } else {
        snprintf(uri, sizeof(uri), "geo:0,0?q=%s", query);
    }
    return open_geo_uri(uri);
}

bool hal_maps_route(const char *from, const char *to) {
    if (!from || !to) return false;
    char uri[2048];
    snprintf(uri, sizeof(uri), 
             "https://www.google.com/maps/dir/%s/%s", from, to);
    return open_geo_uri(uri);
}

#endif // HAL_NO_MAPS
