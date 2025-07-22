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

#ifndef HAL_NO_CLIPBOARD
#include "../../../clipboard.h"
#include "../../internal.h"
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

static const int XA_STRING = 31;
static Display *display;
static Window window;
static Atom UTF8;
Atom targets_atom, text_atom, UTF8, XA_ATOM = 4;

static void _init(void) {
    display = XOpenDisplay(0);
    int N = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, N), 0, 0, 1, 1, 0,
                                 BlackPixel(display, N), WhitePixel(display, N));
}

bool hal_clipboard_available(void) {
    if (!display)
        _init();
    return true;
}

static char* _paste(Atom atom) {
    XEvent event;
    int format;
    unsigned long N, size;
    char *data, *s = 0;
    Atom target,
    CLIPBOARD = XInternAtom(display, "CLIPBOARD", 0),
    XSEL_DATA = XInternAtom(display, "XSEL_DATA", 0);
    XConvertSelection(display, CLIPBOARD, atom, XSEL_DATA, window, CurrentTime);
    XSync(display, 0);
    XNextEvent(display, &event);

    switch(event.type) {
        case SelectionNotify:
            if(event.xselection.selection != CLIPBOARD) break;
            if(event.xselection.property) {
                XGetWindowProperty(event.xselection.display, event.xselection.requestor,
                                   event.xselection.property, 0L,(~0L), 0, AnyPropertyType, &target,
                                   &format, &size, &N,(unsigned char**)&data);
                if(target == UTF8 || target == XA_STRING) {
                    s = strndup(data, size);
                    XFree(data);
                }
                XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
            }
    }
    return s;
}

const char *hal_clipboard_get(void) {
    if (!display)
        _init();
    char *c = 0;
    UTF8 = XInternAtom(display, "UTF8_STRING", True);
    if (UTF8 != None)
        c = _paste(UTF8);
    if (!c)
        c = _paste(XA_STRING);
    return strdup(c);
}

void hal_clipboard_set(const char *str) {
    if (!display)
        _init();

    XEvent event;
    Window owner;
    XSetSelectionOwner(display, selection, window, 0);
    if (XGetSelectionOwner(display, selection) != window)
        return;
    while (1) {
        XNextEvent (display, &event);
        switch (event.type) {
            case SelectionRequest:
                if (event.xselectionrequest.selection != selection)
                    break;
                XSelectionRequestEvent * xsr = &event.xselectionrequest;
                XSelectionEvent ev = {0};
                int R = 0;
                ev.type = SelectionNotify, ev.display = xsr->display, ev.requestor = xsr->requestor,
                ev.selection = xsr->selection, ev.time = xsr->time, ev.target = xsr->target, ev.property = xsr->property;
                if (ev.target == targets_atom)
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_ATOM, 32,
                                        PropModeReplace, (unsigned char*)&UTF8, 1);
                else if (ev.target == XA_STRING || ev.target == text_atom)
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8, PropModeReplace, text, size);
                else if (ev.target == UTF8)
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8, 8, PropModeReplace, text, size);
                else
                    ev.property = None;
                if ((R & 2) == 0)
                    XSendEvent(display, ev.requestor, 0, 0, (XEvent *)&ev);
                break;
            case SelectionClear:
                return;
        }
    }
}
#endif // HAL_NO_CLIPBOARD
