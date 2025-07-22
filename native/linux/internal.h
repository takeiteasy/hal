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

#ifndef PAUL_LINUX_INTERNAL_H
#define PAUL_LINUX_INTERNAL_H
#ifndef __has_include(x)
#define __has_include(x) 1
#endif

#if !defined(PAUL_NO_GTK) && __has_include(<gtk/gtk.h>)
#define PAUL_HAS_GTK
#endif

#if !defined(PAUL_NO_WAYLAND) && __has_include(<wayland-client.h>) && __has_include(<wayland-util.h>)
#define PAUL_HAS_WAYLAND
#endif

#if !defined(PAUL_NO_X11) && __has_include(<X11/Xlib.h>)
#define PAUL_HAS_WAYLAND
#endif
#endif // PAUL_LINUX_INTERNAL_H
