 prefix=/usr/local

all: dim dimcl

dim:
	gcc -g -o dim dim.c -lX11 -std=c99

blur_cl:
	gcc -o blur_cl blurcl.c -std=c99

install:
	install -m 0755 dim $(prefix)/bin
	install -m 0755 dimcl $(prefix)/bin

clean:
	rm dim dimcl