#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "data.h"

#define MSGSIZE 16

static char *fifo = "/tmp/blur_fifo";

int main(int argc, char *argv[])
{

	char *cmd;
	char *cl = "#2a4d58";
	char *op = "0.4";
	char msg[MSGSIZE]; /* 2 command + | + 7 colour + | + 4 op */
	memset(msg, '\0', MSGSIZE);
	
	for(int i = 1; i < argc; i++) {

		if(!strcmp(argv[i], "create"))
		    cmd = "cr";
		else if(!strcmp(argv[i], "close"))
			cmd = "cl";
		else if(!strcmp(argv[i], "exit")) 
			cmd = "ex";
		else if(!strcmp(argv[i], "-d"))
			cl = argv[++i];
		else if(!strcmp(argv[i], "-o"))
			op = argv[++i];
		else {
			printf("Invalid comand %s\n", argv[i]);
			return(0);
		}
		
        }

	/* construct msg */
	strcat(msg,cmd);

	if(cl) {
		strcat(msg,"|");
		strcat(msg,cl);
	}
	if(op) {
		strcat(msg,"|");
		strcat(msg,op);
	}

	/* open fifo and write*/
	int out_fd=open(fifo, O_WRONLY);
	
	write(out_fd, msg, strlen(msg));
	close(out_fd);
	return(0);
}
