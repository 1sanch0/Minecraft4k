#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#define panic(msg)        \
  do {                    \
    fprintf(stderr, msg); \
    exit(EXIT_FAILURE);   \
  } while (0)


#define MC_WIDTH 214 
#define MC_HEIGHT 120

extern uint32_t MC_framebuffer[MC_WIDTH * MC_HEIGHT];

void MC_sleepMillis(uint32_t ms);
uint32_t MC_currentTimeMillis(void);



static uint64_t seed;

static
void setSeed(uint64_t s) {
  seed = (s ^ 0x5DEECE66DULL) & ((1ULL << 48) - 1);
}

static
int next(int bits) {
  seed = (seed * 0x5DEECE66DULL + 0xBULL) & ((1ULL << 48) - 1);
  return (int)(seed >> (48 - bits));
}

static
int nextInt(int bound) {
  if (bound <= 0) panic("nextInt: bound must be positive");

  if ((bound & -bound) == bound) // i.e., bound is a power of 2
    return (int)((bound * (uint64_t)next(31)) >> 31);

  int bits, val;
  do {
    bits = next(31);
    val = bits % bound;
  } while (bits - val + (bound-1) < 0);
  return val;
}


void MC_init(void) {
  setSeed(18295169L);
  printf("Nextint 8: %d\n", nextInt(8));
}

void MC_run(void) {
  for (int i = 0; i < MC_WIDTH; i++)
    for (int j = 0; j < MC_HEIGHT; j++)
      MC_framebuffer[i * MC_HEIGHT + j] = nextInt(0xFFFFFF);
}

void MC_handleEvent(void) {}
