CC = gcc
CCFLAGS = -c -Wall -Wextra -Wvla -std=gnu99 -g
TARFILES = CMakeLists.txt config.h serial_io.h serial_io_linux.c README.md Makefile main.c

# add your .c files here  (no file suffixes)
CLASSES = serial_io_linux main

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
	tar cvf ex3.tar $(TARFILES)

depend:
	makedepend -- $(CCFLAGS) -- $(SRCS)
# DO NOT DELETE

