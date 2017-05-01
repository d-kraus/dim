#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "data.h"

#define OPAQUE 0xffffffff
#define OPACITY "_NET_WM_WINDOW_OPACITY"
#define MIN(a,b)              ((a) < (b) ? (a) : (b))
#define MAX(a,b)              ((a) > (b) ? (a) : (b))

#define BUFFER 16

#define DIMCL "#2a4d58"

static char *fifo = "/tmp/blur_fifo"; 

static double dimopacity = 0.4;
static const char *dimcolor = "#2a4d58";
static char *dimname = "blur";
static char *class = "Blur";

volatile sig_atomic_t stop;

void inthand()
{
	stop = 1;
}

int catcher( Display *disp, XErrorEvent *xe )
{
        return 0;
}

void create(Container *cont)
{
	int screen = DefaultScreen(cont->disp);
	Screen *defScreen = DefaultScreenOfDisplay(cont->disp);
	int dimx, dimy, dimw, dimh;
	Window root = RootWindow(cont->disp, screen);
	XSetWindowAttributes swa;

	dimx = 0;
	dimy = 0;
	dimw = WidthOfScreen(defScreen); 
	dimh = HeightOfScreen(defScreen);

	swa.override_redirect = True;
	swa.background_pixel = cont->dimcol;
	swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;
	cont->win = XCreateWindow(cont->disp, root, dimx, dimy, dimw, dimh, 0,
			    DefaultDepth(cont->disp, screen), CopyFromParent,
			    DefaultVisual(cont->disp, screen),
			    CWOverrideRedirect | CWBackPixel | CWEventMask, &swa);
	
	XClassHint dimhint = { .res_name = dimname, .res_class = class };
	XSetClassHint(cont->disp, cont->win, &dimhint);
  
	dimopacity = MIN(MAX(dimopacity, 0), 1);
	unsigned int dimopacity_set = (unsigned int)(dimopacity * OPAQUE);
	XChangeProperty(cont->disp, cont->win, XInternAtom(cont->disp, OPACITY, False),
			XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) &dimopacity_set, 1L);
	
	XMapRaised(cont->disp, cont->win);
}

void handle_create(Container *cont)
{

	/* check if a window exists and close it  */
	XDestroyWindow(cont->disp, cont->win);
	/* check for color */
	char *cl = strtok(NULL, "|");
	

	if(cl)
		dimcolor = cl;
	else
		dimcolor = DIMCL;

	/* check for opacity */
	char *op = strtok(NULL, "|");
	if(op) {
		dimopacity = atof(op);
	}
	else
		dimopacity = 0.4;

	/* initialize the color */
	cont->cmap = DefaultColormap(cont->disp, DefaultScreen(cont->disp));
	XAllocNamedColor(cont->disp, cont->cmap, dimcolor, &cont->color, &cont->color);
	cont->dimcol = cont->color.pixel;
	create(cont);
}


int main(int argc, char *argv[])
{
	/* register handler */
	signal(SIGINT, inthand);
	XSetErrorHandler( catcher );

	/* local vars */
	XColor color;
	Colormap cmap;
	unsigned long dimcol;
	XEvent ev;

	Container *cont = malloc(sizeof(Container));

	
	/* parse parameters */
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-up"))
			dimopacity = atof(argv[++i]);
		
	}

	/* get display*/
	cont->disp = XOpenDisplay(NULL);

	/* initialize the color */
	cont->cmap = DefaultColormap(cont->disp, DefaultScreen(cont->disp));
	XAllocNamedColor(cont->disp, cmap, dimcolor, &color, &color);

	cont->dimcol = color.pixel;

	/* FIFO */
	mkfifo(fifo, 0666); 
	
	char in[BUFFER];

	/* loop, so the window stays open, wait for the signal */
	while(!stop) {

		/* prepare buffer */
		memset(in, '\0', BUFFER);

		/*open fifo and read */
		int in_fd = open(fifo, O_RDONLY);
		read(in_fd, in, BUFFER);
		close(in_fd);

		char *cmd = strtok(in,"|");

		if(!cmd)
			continue;

		/* (ex)it (cr)eate (cl)ose */
		if(!strcmp("ex",cmd))
			break;
		else if(!strcmp("cr",cmd))
			handle_create(cont);
		else if(!strcmp("cl",in))
			XDestroyWindow(cont->disp, cont->win);

		while(XPending(cont->disp) > 0) {
			XNextEvent(cont->disp, &ev);
		
		}
	}

    XDestroyWindow(cont->disp, cont->win);
    free(cont);
	exit(EXIT_SUCCESS);
}
