SHELL	= /bin/sh

CC	= gcc
CFLAGS	= -pipe -g -Wall -std=c99 -D_GNU_SOURCE 
LDFLAGS	= -lm -lGLU -lglut

ALL = project

all: $(ALL)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^
	
project: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	
valgrind: project
	valgrind --leak-check=full ./project

clean:
	rm -f a.out *.o *~ $(ALL) *.tar.bz2 *.tar.gz Z*
