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

// Android wifi using WifiManager via JNI

#ifndef HAL_NO_WIFI
#include "hal/wifi.h"
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

static jobject get_wifi_manager(JNIEnv *env) {
    jclass contextClass = (*env)->FindClass(env, "android/content/Context");
    jfieldID wifiField = (*env)->GetStaticFieldID(env, contextClass, "WIFI_SERVICE",
        "Ljava/lang/String;");
    jstring wifiService = (*env)->GetStaticObjectField(env, contextClass, wifiField);
    
    jclass activityClass = (*env)->GetObjectClass(env, g_activity);
    jmethodID getSystemService = (*env)->GetMethodID(env, activityClass, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    
    return (*env)->CallObjectMethod(env, g_activity, getSystemService, wifiService);
}

bool hal_wifi_available(void) {
    JNIEnv *env = get_jni_env();
    return env != NULL && get_wifi_manager(env) != NULL;
}

bool hal_wifi_is_enabled(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID isEnabled = (*env)->GetMethodID(env, wifiClass, "isWifiEnabled", "()Z");
    
    return (*env)->CallBooleanMethod(env, wifiManager, isEnabled);
}

bool hal_wifi_enable(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID setEnabled = (*env)->GetMethodID(env, wifiClass, "setWifiEnabled", "(Z)Z");
    
    return (*env)->CallBooleanMethod(env, wifiManager, setEnabled, JNI_TRUE);
}

bool hal_wifi_disable(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID setEnabled = (*env)->GetMethodID(env, wifiClass, "setWifiEnabled", "(Z)Z");
    
    return (*env)->CallBooleanMethod(env, wifiManager, setEnabled, JNI_FALSE);
}

bool hal_wifi_start_scan(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID startScan = (*env)->GetMethodID(env, wifiClass, "startScan", "()Z");
    
    return (*env)->CallBooleanMethod(env, wifiManager, startScan);
}

int hal_wifi_get_networks(hal_wifi_network_t *networks, int max_count) {
    if (!networks || max_count <= 0) return 0;
    
    JNIEnv *env = get_jni_env();
    if (!env) return 0;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return 0;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID getScanResults = (*env)->GetMethodID(env, wifiClass, "getScanResults",
        "()Ljava/util/List;");
    
    jobject resultList = (*env)->CallObjectMethod(env, wifiManager, getScanResults);
    if (!resultList) return 0;
    
    jclass listClass = (*env)->GetObjectClass(env, resultList);
    jmethodID size = (*env)->GetMethodID(env, listClass, "size", "()I");
    jmethodID get = (*env)->GetMethodID(env, listClass, "get", "(I)Ljava/lang/Object;");
    
    int count = (*env)->CallIntMethod(env, resultList, size);
    if (count > max_count) count = max_count;
    
    jclass scanResultClass = (*env)->FindClass(env, "android/net/wifi/ScanResult");
    jfieldID ssidField = (*env)->GetFieldID(env, scanResultClass, "SSID", "Ljava/lang/String;");
    jfieldID bssidField = (*env)->GetFieldID(env, scanResultClass, "BSSID", "Ljava/lang/String;");
    jfieldID levelField = (*env)->GetFieldID(env, scanResultClass, "level", "I");
    jfieldID capsField = (*env)->GetFieldID(env, scanResultClass, "capabilities", "Ljava/lang/String;");
    
    for (int i = 0; i < count; i++) {
        jobject result = (*env)->CallObjectMethod(env, resultList, get, i);
        
        jstring jssid = (*env)->GetObjectField(env, result, ssidField);
        if (jssid) {
            const char *ssid = (*env)->GetStringUTFChars(env, jssid, NULL);
            strncpy(networks[i].ssid, ssid, 63);
            networks[i].ssid[63] = '\0';
            (*env)->ReleaseStringUTFChars(env, jssid, ssid);
        } else {
            networks[i].ssid[0] = '\0';
        }
        
        jstring jbssid = (*env)->GetObjectField(env, result, bssidField);
        if (jbssid) {
            const char *bssid = (*env)->GetStringUTFChars(env, jbssid, NULL);
            strncpy(networks[i].bssid, bssid, 17);
            networks[i].bssid[17] = '\0';
            (*env)->ReleaseStringUTFChars(env, jbssid, bssid);
        } else {
            networks[i].bssid[0] = '\0';
        }
        
        networks[i].signal_strength = (*env)->GetIntField(env, result, levelField);
        
        jstring jcaps = (*env)->GetObjectField(env, result, capsField);
        if (jcaps) {
            const char *caps = (*env)->GetStringUTFChars(env, jcaps, NULL);
            networks[i].is_secure = strstr(caps, "WPA") != NULL || 
                                    strstr(caps, "WEP") != NULL;
            (*env)->ReleaseStringUTFChars(env, jcaps, caps);
        } else {
            networks[i].is_secure = false;
        }
    }
    
    return count;
}

bool hal_wifi_connect(const char *ssid, const char *password) {
    (void)ssid; (void)password;
    // Android 10+ requires using NetworkRequest API which is more complex
    // This simplified version would need additional Java helper code
    return false;
}

bool hal_wifi_disconnect(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID disconnect = (*env)->GetMethodID(env, wifiClass, "disconnect", "()Z");
    
    return (*env)->CallBooleanMethod(env, wifiManager, disconnect);
}

bool hal_wifi_is_connected(void) {
    JNIEnv *env = get_jni_env();
    if (!env) return false;
    
    jobject wifiManager = get_wifi_manager(env);
    if (!wifiManager) return false;
    
    jclass wifiClass = (*env)->GetObjectClass(env, wifiManager);
    jmethodID getConnectionInfo = (*env)->GetMethodID(env, wifiClass, "getConnectionInfo",
        "()Landroid/net/wifi/WifiInfo;");
    
    jobject wifiInfo = (*env)->CallObjectMethod(env, wifiManager, getConnectionInfo);
    if (!wifiInfo) return false;
    
    jclass wifiInfoClass = (*env)->GetObjectClass(env, wifiInfo);
    jmethodID getNetworkId = (*env)->GetMethodID(env, wifiInfoClass, "getNetworkId", "()I");
    
    return (*env)->CallIntMethod(env, wifiInfo, getNetworkId) != -1;
}

#endif // HAL_NO_WIFI
