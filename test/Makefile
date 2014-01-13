CC=gcc
#CFLAGS=-g -std=c99 -pedantic -save-temps -Wall -Wextra -Wshadow -Winline -fmudflap
#CFLAGS=-g -std=c99 -pedantic -Wall -Wextra -Wshadow -Winline
CFLAGS=-O3 -save-temps -D_GNU_SOURCE -std=c99 -pedantic -Wall -Wextra -Wshadow -Winline
LIBS=-lz

TEST= \
test_getline \
test_forline \
test_fortok \
test_sizeof \
test_hash \
test_fnv1a \
test_wc \
test_strset \
test_wfreq \
test_bigram \
test_symtable \
test_darr_t \
test_mallinfo

all: ${TEST}

test_%: test_%.o dlib.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c dlib.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm *.o *.i *.s *~
	-rm ${TEST}
