#include <stdint.h>
#include <math.h>

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



struct {
  int M[32767];
} this;

int arrayOfInt2[262144];
int arrayOfInt3[12288];
long l; 

void MC_init(void) {
  setSeed(18295169L);

  for (int i = 0; i < 262144; i++)
    arrayOfInt2[i] = i / 64 % 64 > 32 + nextInt(8) ? nextInt(8) + 1 : 0;
  
  int j = 1;
  while (j < 16) {
    int k = 255 - nextInt(96);
    int m = 0;
    while (m < 48) {
      int n = 0;
      while (n < 16) {
        int i3;
        int i2;
        int i1 = 9858122;
        if (j == 4) {
          i1 = 0x7F7F7F;
        }
        if (j != 4 || nextInt(3) == 0) {
          k = 255 - nextInt(96);
        }
        if (j == 1 && m < (n * n * 3 + n * 81 >> 2 & 3) + 18) {
          i1 = 6990400;
        } else if (j == 1 && m < (n * n * 3 + n * 81 >> 2 & 3) + 19) {
          k = k * 2 / 3;
        }
        if (j == 7) {
          i1 = 6771249;
          if (n > 0 && n < 15 && (m > 0 && m < 15 || m > 32 && m < 47)) {
            i1 = 12359778;
            i2 = n - 7;
            i3 = (m & 0xF) - 7;
            if (i2 < 0) {
                i2 = 1 - i2;
            }
            if (i3 < 0) {
                i3 = 1 - i3;
            }
            if (i3 > i2) {
                i2 = i3;
            }
            k = 196 - nextInt(32) + i2 % 3 * 32;
          } else if (nextInt(2) == 0) {
            k = k * (150 - (n & 1) * 100) / 100;
          }
        }
        if (j == 5) {
          i1 = 11876885;
          if ((n + m / 4 * 4) % 8 == 0 || m % 4 == 0) {
            i1 = 12365733;
          }
        }
        i2 = k;
        if (m >= 32) {
          i2 /= 2;
        }
        if (j == 8) {
          i1 = 5298487;
          if (nextInt(2) == 0) {
            i1 = 0;
            i2 = 255;
          }
        }
        arrayOfInt3[n + m * 16 + j * 256 * 3] = i3 = (i1 >> 16 & 0xFF) * i2 / 255 << 16 | (i1 >> 8 & 0xFF) * i2 / 255 << 8 | (i1 & 0xFF) * i2 / 255;
        ++n;
      }
      ++m;
    }
    ++j;
  }

  l = MC_currentTimeMillis();
}

float f1 = 96.5f;
float f2 = 65.0f;
float f3 = 96.5f;
float f4 = 0.0f;
float f5 = 0.0f;
float f6 = 0.0f;
int i4 = -1;
int i5 = 0;
float f7 = 0.0f;
float f8 = 0.0f;

void MC_run(void) {
  int j;
  float f9 = (float)sin(f7);
  float f10 = (float)cos(f7);
  float f11 = (float)sin(f8);
  float f12 = (float)cos(f8);

  block7:
    while (MC_currentTimeMillis() - l > 10L) {
      float f14;
      float f13;
      if (this.M[2] > 0) {
          f13 = (float)(this.M[2] - 428) / 214.0f * 2.0f;
          f14 = (float)(this.M[3] - 240) / 120.0f * 2.0f;
          float f15 = (float)sqrt(f13 * f13 + f14 * f14) - 1.2f;
          if (f15 < 0.0f) {
              f15 = 0.0f;
          }
          if (f15 > 0.0f) {
              f7 += f13 * f15 / 400.0f;
              if ((f8 -= f14 * f15 / 400.0f) < -1.57f) {
                  f8 = -1.57f;
              }
              if (f8 > 1.57f) {
                  f8 = 1.57f;
              }
          }
      }
      l += 10L;
      f13 = 0.0f;
      f14 = 0.0f;
      f4 *= 0.5f;
      f5 *= 0.99f;
      f6 *= 0.5f;
      f4 += f9 * (f14 += (float)(this.M[119] - this.M[115]) * 0.02f) + f10 * (f13 += (float)(this.M[100] - this.M[97]) * 0.02f);
      f6 += f10 * f14 - f9 * f13;
      f5 += 0.003f;
      int i8 = 0;
      while (i8 < 3) {
          float f16 = f1 + f4 * (float)((i8 + 0) % 3 / 2);
          float f17 = f2 + f5 * (float)((i8 + 1) % 3 / 2);
          float f19 = f3 + f6 * (float)((i8 + 2) % 3 / 2);
          int i12 = 0;
          while (i12 < 12) {
              int i13 = (int)(f16 + (float)(i12 >> 0 & 1) * 0.6f - 0.3f) - 64;
              int i14 = (int)(f17 + (float)((i12 >> 2) - 1) * 0.8f + 0.65f) - 64;
              int i15 = (int)(f19 + (float)(i12 >> 1 & 1) * 0.6f - 0.3f) - 64;
              if (i13 < 0 || i14 < 0 || i15 < 0 || i13 >= 64 || i14 >= 64 || i15 >= 64 || arrayOfInt2[i13 + i14 * 64 + i15 * 4096] > 0) {
                  if (i8 != 1) goto block7;//continue block7;
                  if (this.M[32] > 0 && f5 > 0.0f) {
                      this.M[32] = 0;
                      f5 = -0.1f;
                      goto block7;// continue block7;
                  }
                  f5 = 0.0f;
                  goto block7;// continue block7;
              }
              ++i12;
          }
          f1 = f16;
          f2 = f17;
          f3 = f19;
          ++i8;
      }
  }
  int i6 = 0;
  int i7 = 0;
  if (this.M[1] > 0 && i4 > 0) {
    arrayOfInt2[i4] = 0;
    this.M[1] = 0;
  }
  if (this.M[0] > 0 && i4 > 0) {
    arrayOfInt2[i4 + i5] = 1;
    this.M[0] = 0;
  }
  int i8 = 0;
  while (i8 < 12) {
    int i9 = (int)(f1 + (float)(i8 >> 0 & 1) * 0.6f - 0.3f) - 64;
    int i10 = (int)(f2 + (float)((i8 >> 2) - 1) * 0.8f + 0.65f) - 64;
    j = (int)(f3 + (float)(i8 >> 1 & 1) * 0.6f - 0.3f) - 64;
    if (i9 >= 0 && i10 >= 0 && j >= 0 && i9 < 64 && i10 < 64 && j < 64) {
      arrayOfInt2[i9 + i10 * 64 + j * 4096] = 0;
    }
    ++i8;
  }
  float i27 = -1.0f;
  int i = 0;
  while (i < MC_WIDTH) {
      float x = (float)(i - (MC_WIDTH / 2)) / 90.0f;
      j = 0;
      while (j < MC_HEIGHT) {
          float y = (float)(j - (MC_HEIGHT / 2)) / 90.0f;
          float f21 = 1.0f;
          float f22 = f21 * f12 + y * f11;
          float f23 = y * f12 - f21 * f11;
          float f24 = x * f10 + f22 * f9;
          float f25 = f22 * f10 - x * f9;

          int i16 = 0;
          int i17 = 255;
          double d = 20.0;
          float f26 = 5.0f;
          int red = 0;
          while (red < 3) {
              float f27 = f24;
              if (red == 1) {
                f27 = f23;
              }
              if (red == 2) {
                f27 = f25;
              }
              float f28 = 1.0f / (f27 < 0.0f ? -f27 : f27);
              float f29 = f24 * f28;
              float f30 = f23 * f28;
              float f31 = f25 * f28;
              float f32 = f1 - (float)((int)f1);
              if (red == 1) {
                f32 = f2 - (float)((int)f2);
              }
              if (red == 2) {
                f32 = f3 - (float)((int)f3);
              }
              if (f27 > 0.0f) {
                f32 = 1.0f - f32;
              }
              float f33 = f28 * f32;
              float f34 = f1 + f29 * f32;
              float f35 = f2 + f30 * f32;
              float f36 = f3 + f31 * f32;
              if (f27 < 0.0f) {
                if (red == 0) {
                  f34 -= 1.0f;
                }
                if (red == 1) {
                  f35 -= 1.0f;
                }
                if (red == 2) {
                  f36 -= 1.0f;
                }
              }
              while ((double)f33 < d) {
                  int i21 = (int)f34 - 64;
                  int i22 = (int)f35 - 64;
                  int i23 = (int)f36 - 64;
                  if (i21 < 0 || i22 < 0 || i23 < 0 || i21 >= 64 || i22 >= 64 || i23 >= 64) break;
                  int i24 = i21 + i22 * 64 + i23 * 4096;
                  int i25 = arrayOfInt2[i24];
                  if (i25 > 0) {
                      i6 = (int)((f34 + f36) * 16.0f) & 0xF;
                      i7 = ((int)(f35 * 16.0f) & 0xF) + 16;
                      if (red == 1) {
                          i6 = (int)(f34 * 16.0f) & 0xF;
                          i7 = (int)(f36 * 16.0f) & 0xF;
                          if (f30 < 0.0f) {
                              i7 += 32;
                          }
                      }
                      int i26 = 0xFFFFFF;
                      if (i24 != i4 || i6 > 0 && i7 % 16 > 0 && i6 < 15 && i7 % 16 < 15) {
                          i26 = arrayOfInt3[i6 + i7 * 16 + i25 * 256 * 3];
                      }
                      if (f33 < f26 && i == this.M[2] / 4 && j == this.M[3] / 4) {
                          i27 = i24;
                          i5 = 1;
                          if (f27 > 0.0f) {
                              i5 = -1;
                          }
                          i5 <<= 6 * red;
                          f26 = f33;
                      }
                      if (i26 > 0) {
                          i16 = i26;
                          i17 = 255 - (int)(f33 / 20.0f * 255.0f);
                          i17 = i17 * (255 - (red + 2) % 3 * 50) / 255;
                          d = f33;
                      }
                  }
                  f34 += f29;
                  f35 += f30;
                  f36 += f31;
                  f33 += f28;
              }
              ++red;
          }
          red = (i16 >> 16 & 0xFF) * i17 / 255;
          int green = (i16 >> 8 & 0xFF) * i17 / 255;
          int blue = (i16 & 0xFF) * i17 / 255;
          MC_framebuffer[i + j * MC_WIDTH] = red << 16 | green << 8 | blue;
          ++j;
      }
      ++i;
  }
  i4 = (int)i27;
  MC_sleepMillis(2);
}

void MC_handleEvent(int id, int key, int x, int y, int modifiers) {
  // https://docs.oracle.com/javase/8/docs/api/constant-values.html#java.awt.Event.F8
  int i = 0;
  switch (id) {
    case 401: i = 1;            // KEY_PRESS
    case 402: this.M[key] = i;  // KEY_RELEASE
              break;
    case 501: i = 1;            // MOUSE_DOWN
              this.M[2] = x;
              this.M[3] = y;
    case 502: {                 // MOUSE_UP
      if ((modifiers & 4) > 0) {// META_MASK??? right button pressed or released
        this.M[1] = i;
        break;
      }
      this.M[0] = i;
      break;
    }
    case 503:                   // MOUSE_MOVE
    case 506: this.M[2] = x;    // MOUSE_DRAG
              this.M[3] = y;
              break;

    case 505: this.M[2] = 0;    // MOUSE_EXIT
  }
}
