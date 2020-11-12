CC = gcc
CCFLAGS = -c -Wall -Wextra -Wvla -std=gnu99 -g
TARFILES = CMakeLists.txt HTTP_client.c HTTP_client.h socket.h socket_linux.c README.md Makefile main.c

# add your .c files here  (no file suffixes)
CLASSES = socket_linux HTTP_client main

# Prepare object and source file list using pattern substitution func.
OBJS = $(patsubst %, %.o,  $(CLASSES))
SRCS = $(patsubst %, %.c, $(CLASSES))

all: main
	$(CC) $(OBJS) -o main

main: $(SRCS)
	$(CC) $(CCFLAGS) $(SRCS)

%.o: %.c
	$(CC) $(CCFLAGS) $*.c

clean:
	rm -f *.o main

tar:
	tar cvf ex1.tar $(TARFILES)

depend:
	makedepend -- $(CCFLAGS) -- $(SRCS)
# DO NOT DELETE

