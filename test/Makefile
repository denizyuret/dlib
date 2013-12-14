CC=gcc
#CFLAGS=-g -std=c99 -pedantic -save-temps -Wall -Wextra -Wshadow -fmudflap
#CFLAGS=-g -std=c99 -pedantic -save-temps -Wall -Wextra -Wshadow
CFLAGS=-g -std=c99 -pedantic -Wall -Wextra -Wshadow

all: foreach_line

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

foreach_line: foreach_line.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm *.o *.i *.s
	-rm foreach_line
