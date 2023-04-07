CC=gcc
CFLAGS=-Wall -Wextra -Werror

all: sshell

sshell: sshell.c
	$(CC) $(CFLAGS) -o sshell sshell.c

clean:
	rm -f sshell