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

// Windows keystore using Credential Manager

#ifndef HAL_NO_KEYSTORE
#include "hal/keystore.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincred.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "advapi32.lib")

static wchar_t *to_wide(const char *str) {
    if (!str) return NULL;
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *wstr = malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
    return wstr;
}

static char *from_wide(const wchar_t *wstr) {
    if (!wstr) return NULL;
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = malloc(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    return str;
}

bool hal_keystore_available(void) {
    return true;
}

bool hal_keystore_set(const char *service, const char *key, const char *value) {
    if (!service || !key || !value) return false;
    
    char target[512];
    snprintf(target, sizeof(target), "%s/%s", service, key);
    wchar_t *wtarget = to_wide(target);
    
    CREDENTIALW cred = {0};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = wtarget;
    cred.CredentialBlobSize = (DWORD)(strlen(value) + 1);
    cred.CredentialBlob = (LPBYTE)value;
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    
    BOOL result = CredWriteW(&cred, 0);
    free(wtarget);
    
    return result;
}

char *hal_keystore_get(const char *service, const char *key) {
    if (!service || !key) return NULL;
    
    char target[512];
    snprintf(target, sizeof(target), "%s/%s", service, key);
    wchar_t *wtarget = to_wide(target);
    
    PCREDENTIALW pcred = NULL;
    BOOL result = CredReadW(wtarget, CRED_TYPE_GENERIC, 0, &pcred);
    free(wtarget);
    
    if (result && pcred) {
        char *value = malloc(pcred->CredentialBlobSize + 1);
        memcpy(value, pcred->CredentialBlob, pcred->CredentialBlobSize);
        value[pcred->CredentialBlobSize] = '\0';
        CredFree(pcred);
        return value;
    }
    
    return NULL;
}

bool hal_keystore_delete(const char *service, const char *key) {
    if (!service || !key) return false;
    
    char target[512];
    snprintf(target, sizeof(target), "%s/%s", service, key);
    wchar_t *wtarget = to_wide(target);
    
    BOOL result = CredDeleteW(wtarget, CRED_TYPE_GENERIC, 0);
    free(wtarget);
    
    return result;
}

#endif // HAL_NO_KEYSTORE
