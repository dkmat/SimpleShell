CC=gcc
CFLAGS=-Wall -Wextra -Werror

sshell: sshell.c helper.h
	$(CC) $(CFLAGS) -o sshell sshell.c helper.h

clean:
	rm -f sshell