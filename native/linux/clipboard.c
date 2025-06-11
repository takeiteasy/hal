/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

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

#ifndef PAUL_NO_CLIPBOARD
#include "../clipboard.h"
#include "internal.h"

#if defined(PAUL_HAS_GTK) && !defined(PAUL_NO_GTK)
#include "backends/gtk/clipboard.c"
#ifndef CLIPBOARD_GTK_UNIMPLEMENTED
#define CLIPBOARD_BACKEND_IMPLEMENTED
#endif
#endif

#if !defined(CLIPBOARD_BACKEND_IMPLEMENTED) && (defined(PAUL_HAS_WAYLAND) && !defined(PAUL_NO_WAYLAND)))
#include "backends/wayland/clipboard.c"
#ifndef CLIPBOARD_WAYLAND_UNIMPLEMENTED
#define CLIPBOARD_BACKEND_IMPLEMENTED
#endif
#endif

#if !defined(CLIPBOARD_BACKEND_IMPLEMENTED) && (defined(PAUL_HAS_X11) && !defined(PAUL_NO_X11)))
#include "backends/x11/clipboard.c"
#ifndef CLIPBOARD_X11_UNIMPLEMENTED
#define CLIPBOARD_BACKEND_IMPLEMENTED
#endif
#endif

#ifndef CLIPBOARD_BACKEND_IMPLEMENTED
#include "backends/dummy/clipboard.c"
#endif
#endif // PAUL_NO_CLIPBOARD
