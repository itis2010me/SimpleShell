all: shell

CC = g++
CFLAGS = -Wall -Werror

shell: shell.cpp 
	$(CC) -o $@ $(CFLAGS) $^
clean:
	rm -f *.o
	rm -f shell