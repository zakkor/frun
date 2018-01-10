#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop.h"

int main(void) {
    const char *dirs[2] = {"/usr/share/applications", "/home/ed/.local/share/applications"};

    struct desktop_entry *entries;
    int entry_count = read_desktop_files_from_dirs(dirs, 2, &entries);





    Display *display;
    Window window;
    XEvent evt;
    char *msg = "Hello, World!";
    int screen;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
    }

    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 100, 100, 1,
                           BlackPixel(display, screen), WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);

    while (1) {
      XNextEvent(display, &evt);
      if (evt.type == Expose) {
        for (int i = 0; i < entry_count; i++) {
            XDrawString(display, window, DefaultGC(display, screen),
                    10, 10 * i,
                    entries[i].name, strlen(entries[i].name));
        }
      }
    }

    XCloseDisplay(display);
    free_all_desktop_entries(&entries, entry_count);
    return 0;
}
