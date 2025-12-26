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

// Windows file chooser using COM IFileOpenDialog/IFileSaveDialog

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

static bool g_com_initialized = false;

static void ensure_com_init(void) {
    if (!g_com_initialized) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        g_com_initialized = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
    }
}

static wchar_t *utf8_to_wide(const char *str) {
    if (str == NULL) return NULL;
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *wstr = malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
    return wstr;
}

static char *wide_to_utf8(const wchar_t *wstr) {
    if (wstr == NULL) return NULL;
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = malloc(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    return str;
}

bool hal_file_chooser_available(void) {
    return true;
}

static COMDLG_FILTERSPEC *build_filters(const char **filters, const char **filter_names, UINT *count) {
    *count = 0;
    if (filters == NULL || filters[0] == NULL)
        return NULL;
    
    // Count filters
    while (filters[*count] != NULL) (*count)++;
    
    COMDLG_FILTERSPEC *specs = malloc(sizeof(COMDLG_FILTERSPEC) * (*count));
    
    for (UINT i = 0; i < *count; i++) {
        specs[i].pszSpec = utf8_to_wide(filters[i]);
        specs[i].pszName = (filter_names && filter_names[i]) 
            ? utf8_to_wide(filter_names[i]) 
            : utf8_to_wide(filters[i]);
    }
    
    return specs;
}

static void free_filters(COMDLG_FILTERSPEC *specs, UINT count) {
    for (UINT i = 0; i < count; i++) {
        free((void *)specs[i].pszSpec);
        free((void *)specs[i].pszName);
    }
    free(specs);
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    if (options == NULL)
        return NULL;
    
    ensure_com_init();
    
    hal_file_chooser_result_t *result = NULL;
    HRESULT hr;
    
    if (options->mode == HAL_FILE_CHOOSER_SAVE) {
        IFileSaveDialog *pfd = NULL;
        hr = CoCreateInstance(&CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IFileSaveDialog, (void **)&pfd);
        if (FAILED(hr)) return NULL;
        
        if (options->title) {
            wchar_t *wtitle = utf8_to_wide(options->title);
            IFileSaveDialog_SetTitle(pfd, wtitle);
            free(wtitle);
        }
        
        if (options->default_name) {
            wchar_t *wname = utf8_to_wide(options->default_name);
            IFileSaveDialog_SetFileName(pfd, wname);
            free(wname);
        }
        
        if (options->default_path) {
            wchar_t *wpath = utf8_to_wide(options->default_path);
            IShellItem *psi = NULL;
            if (SUCCEEDED(SHCreateItemFromParsingName(wpath, NULL, &IID_IShellItem, (void **)&psi))) {
                IFileSaveDialog_SetFolder(pfd, psi);
                IShellItem_Release(psi);
            }
            free(wpath);
        }
        
        UINT filter_count = 0;
        COMDLG_FILTERSPEC *filters = build_filters(options->filters, options->filter_names, &filter_count);
        if (filters) {
            IFileSaveDialog_SetFileTypes(pfd, filter_count, filters);
        }
        
        hr = IFileSaveDialog_Show(pfd, NULL);
        if (SUCCEEDED(hr)) {
            IShellItem *psi = NULL;
            hr = IFileSaveDialog_GetResult(pfd, &psi);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = NULL;
                hr = IShellItem_GetDisplayName(psi, SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr)) {
                    result = malloc(sizeof(hal_file_chooser_result_t));
                    result->count = 1;
                    result->paths = malloc(sizeof(char *) * 2);
                    result->paths[0] = wide_to_utf8(pszPath);
                    result->paths[1] = NULL;
                    CoTaskMemFree(pszPath);
                }
                IShellItem_Release(psi);
            }
        }
        
        if (filters) free_filters(filters, filter_count);
        IFileSaveDialog_Release(pfd);
    } else {
        IFileOpenDialog *pfd = NULL;
        hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IFileOpenDialog, (void **)&pfd);
        if (FAILED(hr)) return NULL;
        
        DWORD dwOptions;
        IFileOpenDialog_GetOptions(pfd, &dwOptions);
        
        if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
            IFileOpenDialog_SetOptions(pfd, dwOptions | FOS_PICKFOLDERS);
        } else {
            if (options->allow_multiple) {
                IFileOpenDialog_SetOptions(pfd, dwOptions | FOS_ALLOWMULTISELECT);
            }
        }
        
        if (options->title) {
            wchar_t *wtitle = utf8_to_wide(options->title);
            IFileOpenDialog_SetTitle(pfd, wtitle);
            free(wtitle);
        }
        
        if (options->default_path) {
            wchar_t *wpath = utf8_to_wide(options->default_path);
            IShellItem *psi = NULL;
            if (SUCCEEDED(SHCreateItemFromParsingName(wpath, NULL, &IID_IShellItem, (void **)&psi))) {
                IFileOpenDialog_SetFolder(pfd, psi);
                IShellItem_Release(psi);
            }
            free(wpath);
        }
        
        if (options->mode != HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
            UINT filter_count = 0;
            COMDLG_FILTERSPEC *filters = build_filters(options->filters, options->filter_names, &filter_count);
            if (filters) {
                IFileOpenDialog_SetFileTypes(pfd, filter_count, filters);
                free_filters(filters, filter_count);
            }
        }
        
        hr = IFileOpenDialog_Show(pfd, NULL);
        if (SUCCEEDED(hr)) {
            if (options->allow_multiple && options->mode != HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
                IShellItemArray *psia = NULL;
                hr = IFileOpenDialog_GetResults(pfd, &psia);
                if (SUCCEEDED(hr)) {
                    DWORD count = 0;
                    IShellItemArray_GetCount(psia, &count);
                    
                    result = malloc(sizeof(hal_file_chooser_result_t));
                    result->count = (int)count;
                    result->paths = malloc(sizeof(char *) * (count + 1));
                    
                    for (DWORD i = 0; i < count; i++) {
                        IShellItem *psi = NULL;
                        IShellItemArray_GetItemAt(psia, i, &psi);
                        PWSTR pszPath = NULL;
                        IShellItem_GetDisplayName(psi, SIGDN_FILESYSPATH, &pszPath);
                        result->paths[i] = wide_to_utf8(pszPath);
                        CoTaskMemFree(pszPath);
                        IShellItem_Release(psi);
                    }
                    result->paths[count] = NULL;
                    IShellItemArray_Release(psia);
                }
            } else {
                IShellItem *psi = NULL;
                hr = IFileOpenDialog_GetResult(pfd, &psi);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = NULL;
                    hr = IShellItem_GetDisplayName(psi, SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr)) {
                        result = malloc(sizeof(hal_file_chooser_result_t));
                        result->count = 1;
                        result->paths = malloc(sizeof(char *) * 2);
                        result->paths[0] = wide_to_utf8(pszPath);
                        result->paths[1] = NULL;
                        CoTaskMemFree(pszPath);
                    }
                    IShellItem_Release(psi);
                }
            }
        }
        
        IFileOpenDialog_Release(pfd);
    }
    
    return result;
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
    UINT mb_type = MB_OK;
    
    // Map alert type to icon
    switch (type) {
        case HAL_ALERT_INFO:
            mb_type |= MB_ICONINFORMATION;
            break;
        case HAL_ALERT_WARNING:
            mb_type |= MB_ICONWARNING;
            break;
        case HAL_ALERT_ERROR:
            mb_type |= MB_ICONERROR;
            break;
        case HAL_ALERT_QUESTION:
            mb_type |= MB_ICONQUESTION;
            break;
    }
    
    // Map button count to type (simplified)
    if (button_count == 2) {
        mb_type = (mb_type & ~MB_OK) | MB_YESNO;
    } else if (button_count == 3) {
        mb_type = (mb_type & ~MB_OK) | MB_YESNOCANCEL;
    }
    
    wchar_t *wtitle = utf8_to_wide(title ? title : "");
    wchar_t *wmessage = utf8_to_wide(message ? message : "");
    
    int result = MessageBoxW(NULL, wmessage, wtitle, mb_type);
    
    free(wtitle);
    free(wmessage);
    
    // Map result to button index
    switch (result) {
        case IDOK:
        case IDYES:
            return 0;
        case IDNO:
            return 1;
        case IDCANCEL:
            return button_count > 2 ? 2 : -1;
        default:
            return -1;
    }
}

#endif // HAL_NO_FILE_CHOOSER
