CC = gcc
CFLAGS = -W -Wall
TARGET = myshell
OBJS = myshell.o commands.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

myshell.o: myshell.c commands.h
	$(CC) $(CFLAGS) -c myshell.c

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c

clean:
	rm -f *.o $(TARGET)