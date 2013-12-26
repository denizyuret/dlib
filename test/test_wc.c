#include "dlib.h"

int main(int argc, char **argv) {
  char *fname = (argc == 1) ? NULL : argv[1];
  msg("Reading %s", fname == NULL ? "stdin" : fname);
  size_t line = 0;
  size_t word = 0;
  size_t byte = 0;
  forline(buf, fname) {
    line++;
    byte += strlen(buf);
    fortok(tok, buf) {
      word++;
    }
  }
  msg("%zu %zu %zu", line, word, byte);
}
