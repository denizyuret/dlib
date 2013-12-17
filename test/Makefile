CC=gcc
#CFLAGS=-g -std=c99 -pedantic -save-temps -Wall -Wextra -Wshadow -Winline -fmudflap
#CFLAGS=-g -std=c99 -pedantic -Wall -Wextra -Wshadow -Winline
CFLAGS=-g -save-temps -D_GNU_SOURCE -std=c99 -pedantic -Wall -Wextra -Wshadow -Winline
LIBS=-lz

all: test_forline test_getline test_fortok

test_%: test_%.o dlib.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c dlib.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm *.o *.i *.s *~
	-rm test_forline test_getline test_fortok
