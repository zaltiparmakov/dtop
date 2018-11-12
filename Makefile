SOURCE_FILE_LIST = dtop.c
TARGET = dtop

CC = gcc

LIBRARIES =

CFLAGS = -lcurses -pthread -std=c99 -D_DEFAULT_SOURCE -Wall  -D_POSIX_C_SOURCE

OBJECTS = $(SOURCE_FILE_LIST:.c=.o)
all: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIBRARIES)
debug: CFLAGS += -g
debug: all
clean:
	rm -f $(TARGET) $(OBJECTS)
