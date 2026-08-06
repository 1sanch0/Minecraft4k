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

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832
#endif
#define DEGREES (M_PI / 180.0)

#define DEAD_ZONE_RADIUS 1.2f

struct {
  int M[32767];
} this;

static float pos_x = 96.5f, pos_y = 65.0f, pos_z = 96.5f;
static float acc_x = 0.0f, acc_y = 0.0f, acc_z = 0.0f;
static float yaw = 0.0f, pitch = 0.0f;

static int mouse_x, mouse_y;

static int world[64*64*64];
static int sprites[16*16*48];
static long l; 

void MC_destroy() {
}

void MC_init(void) {
  setSeed(18295169L);

  for (int i = 0; i < 64*64*64; i++)
    world[i] = i / 64 % 64 > 32 + nextInt(8) ? nextInt(8) + 1 : 0;
  
  for (int j = 1; j < 16; j++) {
    int k = 255 - nextInt(96);
    for (int v = 0; v < 48; v++) {
      for (int u = 0; u < 16; u++) {
        int i3;
        int i2;
        int i1 = 9858122;
        if (j == 4) {
          i1 = 0x7F7F7F;
        }
        if (j != 4 || nextInt(3) == 0) {
          k = 255 - nextInt(96);
        }
        if (j == 1 && v < (u * u * 3 + u * 81 >> 2 & 3) + 18) {
          i1 = 6990400;
        } else if (j == 1 && v < (u * u * 3 + u * 81 >> 2 & 3) + 19) {
          k = k * 2 / 3;
        }
        if (j == 7) {
          i1 = 6771249;
          if (u > 0 && u < 15 && (v > 0 && v < 15 || v > 32 && v < 47)) {
            i1 = 12359778;
            i2 = u - 7;
            i3 = (v & 0xF) - 7;
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
            k = k * (150 - (u & 1) * 100) / 100;
          }
        }
        if (j == 5) {
          i1 = 11876885;
          if ((u + v / 4 * 4) % 8 == 0 || v % 4 == 0) {
            i1 = 12365733;
          }
        }
        i2 = k;
        if (v >= 32) {
          i2 /= 2;
        }
        if (j == 8) {
          i1 = 5298487;
          if (nextInt(2) == 0) {
            i1 = 0;
            i2 = 255;
          }
        }
        sprites[u + v * 16 + j * 256 * 3] = (i1 >> 16 & 0xFF) * i2 / 255 << 16 | (i1 >> 8 & 0xFF) * i2 / 255 << 8 | (i1 & 0xFF) * i2 / 255;
      }
    }
  }

  l = MC_currentTimeMillis();
}

int block_lookat_idx = -1;
int lookat_closer_offset = 0;

void MC_run(void) {
  block7:
    while (MC_currentTimeMillis() - l > 10L) {
      if (mouse_x > 0) {
        float dx = (mouse_x - 2*MC_WIDTH) / (float)MC_WIDTH * 2.0f;
        float dy = (mouse_y - 2*MC_HEIGHT) / (float)MC_HEIGHT * 2.0f;

        float d = sqrtf(dx * dx + dy * dy) - DEAD_ZONE_RADIUS;

        if (d > 0.0f) {
          yaw   += dx * d / 400.0f;
          pitch -= dy * d / 400.0f;
          if (pitch < -90*DEGREES) pitch = -90*DEGREES;
          if (pitch >  90*DEGREES) pitch =  90*DEGREES;
        }
      }

      l += 10L;
      acc_x *= 0.5f;
      acc_y *= 0.99f;
      acc_z *= 0.5f;
      float right = (float)(this.M['d'] - this.M['a']) * 0.02f;
      float left = (float)(this.M['w'] - this.M['s']) * 0.02f;
      acc_x += sinf(yaw) * left + cosf(yaw) * right;
      acc_z += cosf(yaw) * left - sinf(yaw) * right;
      acc_y += 0.003f;

      for (int dim = 0; dim < 3; dim++) {
        float next_x = pos_x + acc_x * (float)(((dim + 0) % 3) / 2);
        float next_y = pos_y + acc_y * (float)(((dim + 1) % 3) / 2);
        float next_z = pos_z + acc_z * (float)(((dim + 2) % 3) / 2);
        for (int m = 0; m < 12; m++) { // Iter lower 2 layers
          int x = (int)(next_x + (float)(m >> 0 & 1) * 0.6f - 0.3f) - 64;
          int y = (int)(next_y + (float)((m >> 2) - 1) * 0.8f + 0.65f) - 64;
          int z = (int)(next_z + (float)(m >> 1 & 1) * 0.6f - 0.3f) - 64;
          if ( x < 0   || y < 0   || z < 0
            || x >= 64 || y >= 64 || z >= 64
            || world[x + y * 64 + z * 4096] > 0) {
            if (dim != 1) goto block7;
            if (this.M[' '] > 0 && acc_y > 0.0f) {
              this.M[' '] = 0;
              acc_y = -0.1f;
              goto block7;
            }
            acc_y = 0.0f;
            goto block7;
          }
        }
        pos_x = next_x;
        pos_y = next_y;
        pos_z = next_z;
      }
    }



  

  if (this.M[1] > 0 && block_lookat_idx > 0) {
    world[block_lookat_idx] = 0;
    this.M[1] = 0;
  }
  if (this.M[0] > 0 && block_lookat_idx > 0) {
    world[block_lookat_idx + lookat_closer_offset] = 1;
    this.M[0] = 0;
  }
  for (int m = 0; m < 12; m++) { // Iter lower 2 layers
    int x = (int)(pos_x + (float)(m >> 0 & 1) * 0.6f - 0.3f) - 64;
    int y = (int)(pos_y + (float)((m >> 2) - 1) * 0.8f + 0.65f) - 64;
    int z = (int)(pos_z + (float)(m >> 1 & 1) * 0.6f - 0.3f) - 64;
    if (x >= 0 && y >= 0 && z >= 0 && x < 64 && y < 64 && z < 64) {
      world[x + y * 64 + z * 4096] = 0;
    }
  }
  
  int u = 0;
  int v = 0;
  float i27 = -1.0f;
  for (int i = 0; i < MC_WIDTH; i++) {
    float x = (float)(i - (MC_WIDTH / 2)) / 90.0f;
    for (int j = 0; j < MC_HEIGHT; j++) {
      float y = (float)(j - (MC_HEIGHT / 2)) / 90.0f;

      float t = 1.0f * cosf(pitch) + y * sinf(pitch);
      float ray_dir_x = x * cosf(yaw) + t * sinf(yaw);
      float ray_dir_y = y * cosf(pitch) - 1.0f * sinf(pitch);
      float ray_dir_z = t * cosf(yaw) - x * sinf(yaw);

      int px_color = 0;
      int fog = 255;
      double d = 20.0;
      float outline_distance = 5.0f;
      for (int dim = 0; dim < 3; dim++) {
        float         dim_component = ray_dir_x;
        if (dim == 1) dim_component = ray_dir_y;
        if (dim == 2) dim_component = ray_dir_z;

        float dim_norm = 1.0f / (dim_component < 0.0f ? -dim_component : dim_component);
        float dx = ray_dir_x * dim_norm;
        float dy = ray_dir_y * dim_norm;
        float dz = ray_dir_z * dim_norm;

        float         fract = pos_x - (float)((int)pos_x);
        if (dim == 1) fract = pos_y - (float)((int)pos_y);
        if (dim == 2) fract = pos_z - (float)((int)pos_z);
        if (dim_component > 0.0f)
          fract = 1.0f - fract;

        float step = dim_norm * fract;
        float look_block_x = pos_x + dx * fract;
        float look_block_y = pos_y + dy * fract;
        float look_block_z = pos_z + dz * fract;
        if (dim_component < 0.0f) {
          if (dim == 0) look_block_x -= 1.0f;
          if (dim == 1) look_block_y -= 1.0f;
          if (dim == 2) look_block_z -= 1.0f;
        }
        while ((double)step < d) {
          int idx_x = (int)look_block_x - 64;
          int idx_y = (int)look_block_y - 64;
          int idx_z = (int)look_block_z - 64;
          if (idx_x < 0 || idx_y < 0 || idx_z < 0 || idx_x >= 64 || idx_y >= 64 || idx_z >= 64)
            break;
          int looking_block_idx = idx_x + idx_y * 64 + idx_z * 4096;
          int block_id = world[looking_block_idx];
          if (block_id > 0) {
            u = (int)((look_block_x + look_block_z) * 16.0f) & 0xF;
            v = ((int)(look_block_y * 16.0f) & 0xF) + 16;
            if (dim == 1) {
              u = (int)(look_block_x * 16.0f) & 0xF;
              v = (int)(look_block_z * 16.0f) & 0xF;
              if (dy < 0.0f)
                v += 32;
            }
            int color = 0xFFFFFF;
            if (looking_block_idx != block_lookat_idx || u > 0 && v % 16 > 0 && u < 15 && v % 16 < 15) {
              color = sprites[u + v * 16 + block_id * 256 * 3];
            }
            // Draws outline
            if (step < outline_distance && i == mouse_x / 4 && j == mouse_y / 4) {
              i27 = looking_block_idx;
              lookat_closer_offset = 1;
              if (dim_component > 0.0f)
                lookat_closer_offset = -1;
              lookat_closer_offset <<= 6 * dim;
              outline_distance = step;
            }
            if (color > 0) {
              px_color = color;
              fog = 255 - (int)(step / 20.0f * 255.0f);
              fog = fog * (255 - (dim + 2) % 3 * 50) / 255;
              d = step;
            }
          }
          look_block_x += dx;
          look_block_y += dy;
          look_block_z += dz;
          step += dim_norm;
        }
      }
      int red = (px_color >> 16 & 0xFF) * fog / 255;
      int green = (px_color >> 8 & 0xFF) * fog / 255;
      int blue = (px_color & 0xFF) * fog / 255;
      MC_framebuffer[j * MC_WIDTH + i] = red << 16 | green << 8 | blue;
    }
  }
  block_lookat_idx = (int)i27;
}

void MC_handleEvent(int id, int key, int x, int y, int modifiers) {
  // https://docs.oracle.com/javase/8/docs/api/constant-values.html#java.awt.Event.F8
  int i = 0;
  switch (id) {
    case 401: i = 1;            // KEY_PRESS
    case 402: this.M[key] = i;  // KEY_RELEASE
              break;
    case 501: i = 1;            // MOUSE_DOWN
              mouse_x = x;
              mouse_y = y;
    case 502: {                 // MOUSE_UP
      if ((modifiers & 4) > 0) {// META_MASK??? right button pressed or released
        this.M[1] = i;
        break;
      }
      this.M[0] = i;
      break;
    }
    case 503:                   // MOUSE_MOVE
    case 506: mouse_x = x;      // MOUSE_DRAG
              mouse_y = y;
              break;

    case 505: mouse_x = 0;    // MOUSE_EXIT
  }
}
