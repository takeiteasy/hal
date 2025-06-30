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

#ifndef HAL_NO_FILE_CHOOSER
#include "../file_chooser.h"
#include "internal.h"

#if defined(HAL_HAS_GTK) && !defined(HAL_NO_GTK)
#include "backends/gtk/file_chooser.c"
#ifndef FILE_CHOOSER_GTK_UNIMPLEMENTED
#define FILE_CHOOSER_BACKEND_IMPLEMENTED
#endif
#endif

#if !defined(FILE_CHOOSER_BACKEND_IMPLEMENTED) && (defined(HAL_HAS_WAYLAND) && !defined(HAL_NO_WAYLAND)))
#include "backends/wayland/file_chooser.c"
#ifndef FILE_CHOOSER_WAYLAND_UNIMPLEMENTED
#define FILE_CHOOSER_BACKEND_IMPLEMENTED
#endif
#endif

#if !defined(FILE_CHOOSER_BACKEND_IMPLEMENTED) && (defined(HAL_HAS_X11) && !defined(HAL_NO_X11)))
#include "backends/x11/file_chooser.c"
#ifndef FILE_CHOOSER_X11_UNIMPLEMENTED
#define FILE_CHOOSER_BACKEND_IMPLEMENTED
#endif
#endif

#ifndef FILE_CHOOSER_BACKEND_IMPLEMENTED
#include "backends/dummy/file_chooser.c"
#endif
#endif // HAL_NO_FILE_CHOOSER
