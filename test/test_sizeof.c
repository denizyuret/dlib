#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct array_s {
  void *dat;
  size_t len;
  size_t cap;
} *array_t;

typedef struct array32_s {
  void *dat;
  uint32_t len;
  uint32_t cap;
} *array32_t;

int main() {
  printf("size_t=%zu\n", sizeof(size_t));
  //  printf("void=%zu\n", sizeof(void));
  printf("bool=%zu\n", sizeof(bool));
  printf("uint8=%zu\n", sizeof(uint8_t));
  printf("uint16=%zu\n", sizeof(uint16_t));
  printf("uint32=%zu\n", sizeof(uint32_t));
  printf("uint64=%zu\n", sizeof(uint64_t));
  printf("array_t=%zu\n", sizeof(array_t));
  printf("array_s=%zu\n", sizeof(struct array_s));
  printf("array32_t=%zu\n", sizeof(array32_t));
  printf("array32_s=%zu\n", sizeof(struct array32_s));
}
