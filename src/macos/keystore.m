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

// macOS/iOS keystore using Keychain

#ifndef HAL_NO_KEYSTORE
#include "hal/keystore.h"
#import <Foundation/Foundation.h>
#import <Security/Security.h>

bool hal_keystore_available(void) {
    return true;
}

bool hal_keystore_set(const char *service, const char *key, const char *value) {
    if (!service || !key || !value) return false;
    
    NSString *nsService = [NSString stringWithUTF8String:service];
    NSString *nsKey = [NSString stringWithUTF8String:key];
    NSData *data = [[NSString stringWithUTF8String:value] dataUsingEncoding:NSUTF8StringEncoding];
    
    // Delete existing item first
    NSDictionary *deleteQuery = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: nsService,
        (__bridge id)kSecAttrAccount: nsKey
    };
    SecItemDelete((__bridge CFDictionaryRef)deleteQuery);
    
    // Add new item
    NSDictionary *addQuery = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: nsService,
        (__bridge id)kSecAttrAccount: nsKey,
        (__bridge id)kSecValueData: data
    };
    
    OSStatus status = SecItemAdd((__bridge CFDictionaryRef)addQuery, NULL);
    return status == errSecSuccess;
}

char *hal_keystore_get(const char *service, const char *key) {
    if (!service || !key) return NULL;
    
    NSString *nsService = [NSString stringWithUTF8String:service];
    NSString *nsKey = [NSString stringWithUTF8String:key];
    
    NSDictionary *query = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: nsService,
        (__bridge id)kSecAttrAccount: nsKey,
        (__bridge id)kSecReturnData: @YES,
        (__bridge id)kSecMatchLimit: (__bridge id)kSecMatchLimitOne
    };
    
    CFDataRef dataRef = NULL;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef *)&dataRef);
    
    if (status == errSecSuccess && dataRef) {
        NSData *data = (__bridge_transfer NSData *)dataRef;
        NSString *str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        return strdup(str.UTF8String);
    }
    
    return NULL;
}

bool hal_keystore_delete(const char *service, const char *key) {
    if (!service || !key) return false;
    
    NSString *nsService = [NSString stringWithUTF8String:service];
    NSString *nsKey = [NSString stringWithUTF8String:key];
    
    NSDictionary *query = @{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: nsService,
        (__bridge id)kSecAttrAccount: nsKey
    };
    
    OSStatus status = SecItemDelete((__bridge CFDictionaryRef)query);
    return status == errSecSuccess;
}

#endif // HAL_NO_KEYSTORE
