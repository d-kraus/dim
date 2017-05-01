#ifndef DATA_H
#define DATA_H

#include <X11/Xlib.h>

typedef struct
{
    Display *disp;
    Window  win;
    Colormap cmap;
    XColor color;
    unsigned long dimcol;
} Container;

#endif