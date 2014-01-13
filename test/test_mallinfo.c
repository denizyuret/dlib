#include "dlib.h"
#include <stdio.h>
#include <malloc.h>

void printmem() {
  struct mallinfo mi = mallinfo(); 
  printf("hblks=%u\thblkhd=%u\tuordblks=%u", mi.hblks, mi.hblkhd, mi.uordblks);
  char *tok[23];
  forline(l, "/proc/self/stat") {
    split(l, ' ', tok, 23); break;
  }
  printf("\tvsize=%s", tok[22]);
  forline(l, "/proc/self/statm") {
    l[strlen(l)-1] = '\0';
    printf("\tstatm=[%s]", l); 
    break;
  }
  forline(l, "/proc/self/status") {
    if (l[0] != 'V') continue;
    l[strlen(l)-1] = '\0';
    printf("\t%s", l); 
  }
  printf("\n");
  malloc_stats();
}

int main() {
  struct mallinfo mi;
  printmem();
  void *p0 = malloc(300);
  printmem();
  void *p1 = malloc(1ULL << 30);
  printmem();
  void *p2 = malloc(1ULL << 30);
  printmem();
  void *p3 = malloc(1ULL << 30);
  printmem();
  void *p4 = malloc(1ULL << 30);
  printmem();
  void *p5 = malloc(1ULL << 30);
  printmem();
  void *p6 = malloc(100);
  printmem();
  free(p5);
  printmem();
  free(p4);
  printmem();
  free(p3);
  printmem();
  free(p2);
  printmem();
  free(p1);
  printmem();
  free(p6);
  printmem();
}
