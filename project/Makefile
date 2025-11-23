CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline
TARGET = myshell
OBJS = myshell.o commands.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

myshell.o: myshell.c commands.h
	$(CC) $(CFLAGS) -c myshell.c

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean