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

typedef struct {
  union {
    float arr[3];
    struct { float x, y, z; };
  };
} vec3_t;

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832
#endif
#define DEGREES (M_PI / 180.0)

#define WORLD_SIZE 64
#define DEAD_ZONE_RADIUS 1.2f
#define HFRICTION 0.5f
#define VFRICTION 0.99f
#define GRAVITY 0.003f
#define SPEED 0.02f
#define JUMP_STRENGH 0.1f

#define BLOCK_AIR   0
#define BLOCK_GRASS 1

#define BLOCK_STONE 4
#define BLOCK_BRICK 5

#define BLOCK_LOG   7
#define BLOCK_LEAF  8

#define COLOR_GRASS  0x6aaa40
#define COLOR_STONE  0x7F7F7F
#define COLOR_BRICK1 0xb53a15
#define COLOR_BRICK2 0xbcafa5
#define COLOR_LOG1   0x675231
#define COLOR_LOG2   0xbc9862
#define COLOR_LEAF   0x50d937

uint32_t *MC_framebuffer;

static vec3_t pos = { .x = 96.5f, .y = 65.0f, .z = 96.5f };
static vec3_t acc = { .x = 00.0f, .y = 00.0f, .z = 00.0f };
static float yaw = 0.0f, pitch = 0.0f;

static int mouse_x, mouse_y;
static int mouse_left, mouse_right;

static uint8_t *keyboard;
static int *world;
static int *sprites;

#define GET_BLOCK_IDX(idx, idy, idz) (idz * WORLD_SIZE*WORLD_SIZE + idy * WORLD_SIZE + idx)
#define GET_PIXEL_IDX(u, v, id) (id * 16*16*3 + v * 16 + u)

static int lookat_block_idx = -1;
static int lookat_closer_offset = 0;


void MC_init(void) {
  setSeed(18295169L);

  MC_framebuffer = calloc(MC_WIDTH * MC_HEIGHT, sizeof(uint32_t));
  keyboard = calloc(128, sizeof(uint8_t));
  world = calloc(WORLD_SIZE*WORLD_SIZE*WORLD_SIZE, sizeof(int));
  sprites = calloc(16*16*16*3, sizeof(int));

  for (int i = 0; i < WORLD_SIZE*WORLD_SIZE*WORLD_SIZE; i++)
    world[i] = i / WORLD_SIZE % WORLD_SIZE > WORLD_SIZE/2 + nextInt(8) ? nextInt(8) + 1 : 0;
  
  for (int block_id = 1; block_id < 16; block_id++) {
    int k = 255 - nextInt(96);
    for (int v = 0; v < 16*3; v++) {
      for (int u = 0; u < 16; u++) {
        int details;
        int color = 0x966c4a;
        if (block_id == BLOCK_STONE)
          color = COLOR_STONE;

        if (block_id != BLOCK_STONE || nextInt(3) == 0)
          k = 255 - nextInt(96);

        if (block_id == BLOCK_GRASS && v < ((u * u * 3 + u * 81) >> 2 & 3) + 18) {
          color = COLOR_GRASS;
        } else if (block_id == BLOCK_GRASS && v < ((u * u * 3 + u * 81) >> 2 & 3) + 19) {
          k = k * 2 / 3;
        }

        if (block_id == BLOCK_LOG) {
          color = COLOR_LOG1;
          if (u > 0 && u < 15 && ((v > 0 && v < 15) || (v > 32 && v < 47))) {
            color = COLOR_LOG2;

            details = u - 7;
            if (details < 0)
              details = 1 - details;

            int i3 = (v & 0xF) - 7;
            if (i3 < 0) i3 = 1 - i3;
            if (i3 > details) details = i3;

            k = 196 - nextInt(32) + details % 3 * 32;
          } else if (nextInt(2) == 0)
            k = k * (150 - (u & 1) * 100) / 100;
        }

        if (block_id == BLOCK_BRICK)
          color = ((u + v / 4 * 4) % 8 == 0 || v % 4 == 0)
                ? COLOR_BRICK2 : COLOR_BRICK1;

        details = k;
        if (v >= 32) details /= 2;

        if (block_id == BLOCK_LEAF) {
          color = COLOR_LEAF;
          if (nextInt(2) == 0) {
            color = 0;
            details = 255;
          }
        }
        
        sprites[GET_PIXEL_IDX(u, v, block_id)] = \
          (color >> 16 & 0xFF) * details / 255 << 16
        | (color >> 8 & 0xFF) * details / 255 << 8
        | (color & 0xFF) * details / 255;
      }
    }
  }

  // FILE *fp = fopen("atlas.bin", "wb");
  // fwrite(sprites, 1, sizeof(sprites), fp);
  // fclose(fp);
}

void MC_destroy(void) {
  free(MC_framebuffer);
  free(keyboard);
  free(world);
  free(sprites);
}

static
void render(void) {
  int u = 0, v = 0;
  int lookat_idx_temp = -1;
  for (int i = 0; i < MC_WIDTH; i++) {
    float x = (i - (MC_WIDTH / 2)) / 90.0f;
    for (int j = 0; j < MC_HEIGHT; j++) {
      float y = (j - (MC_HEIGHT / 2)) / 90.0f;

      float t = 1.0f * cosf(pitch) + y * sinf(pitch);
      vec3_t ray = {
        .x = x * cosf(yaw) + t * sinf(yaw),
        .y = y * cosf(pitch) - 1.0f * sinf(pitch),
        .z = t * cosf(yaw) - x * sinf(yaw),
      };

      int px_color = 0;
      int fog = 255;
      float d = 20.0;
      float outline_distance = 5.0f;
      for (int dim = 0; dim < 3; dim++) {
        float dim_component = ray.arr[dim];

        float dim_norm = 1.0f / (dim_component < 0.0f ? -dim_component : dim_component);
        float dx = ray.x * dim_norm;
        float dy = ray.y * dim_norm;
        float dz = ray.z * dim_norm;

        float fract = pos.arr[dim] - (float)((int)pos.arr[dim]);
        if (dim_component > 0.0f)
          fract = 1.0f - fract;

        float step = dim_norm * fract;
        vec3_t ray_block = {
          .x = pos.x + dx * fract,
          .y = pos.y + dy * fract,
          .z = pos.z + dz * fract,
        };
        if (dim_component < 0.0f)
          ray_block.arr[dim] -= 1.0f;

        while (step < d) {
          int idx_x = ray_block.x - WORLD_SIZE;
          int idx_y = ray_block.y - WORLD_SIZE;
          int idx_z = ray_block.z - WORLD_SIZE;
          if ( idx_x < 0 || idx_x >= WORLD_SIZE
            || idx_y < 0 || idx_y >= WORLD_SIZE
            || idx_z < 0 || idx_z >= WORLD_SIZE)
            break;
          int ray_block_idx = GET_BLOCK_IDX(idx_x, idx_y, idx_z);
          int ray_block_id = world[ray_block_idx];
          if (ray_block_id > 0) {
            u = (int)((ray_block.x + ray_block.z) * 16.0f) & 0xF;
            v = ((int)(ray_block.y * 16.0f) & 0xF) + 16;
            if (dim == 1) {
              u = (int)(ray_block.x * 16.0f) & 0xF;
              v = (int)(ray_block.z * 16.0f) & 0xF;
              if (dy < 0.0f)
                v += 32;
            }

            int color = 0xFFFFFF;

            if ( ray_block_idx != lookat_block_idx
              || (u > 0 && v % 16 > 0 && u < 15 && v % 16 < 15))
              color = sprites[GET_PIXEL_IDX(u, v, ray_block_id)];

            if (step < outline_distance && i == mouse_x / 4 && j == mouse_y / 4) {
              lookat_idx_temp = ray_block_idx;
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
          ray_block.x += dx;
          ray_block.y += dy;
          ray_block.z += dz;
          step += dim_norm;
        }
      }
      int red = (px_color >> 16 & 0xFF) * fog / 255;
      int green = (px_color >> 8 & 0xFF) * fog / 255;
      int blue = (px_color & 0xFF) * fog / 255;
      MC_framebuffer[j * MC_WIDTH + i] = red << 16 | green << 8 | blue;
    }
  }
  lookat_block_idx = lookat_idx_temp;
}

void MC_run(float dt) {
  if (mouse_x || mouse_y) {
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

  float right = (float)(keyboard['d'] - keyboard['a']) * SPEED;
  float left  = (float)(keyboard['w'] - keyboard['s']) * SPEED;

  acc.x += sinf(yaw) * left + cosf(yaw) * right;
  acc.z += cosf(yaw) * left - sinf(yaw) * right;
  acc.y += GRAVITY;

  acc.x *= HFRICTION;
  acc.z *= HFRICTION;
  acc.y *= VFRICTION;

  for (int dim = 0; dim < 3; dim++) {
    float next_x = pos.x + acc.x * (((dim + 0) % 3) / 2);
    float next_y = pos.y + acc.y * (((dim + 1) % 3) / 2);
    float next_z = pos.z + acc.z * (((dim + 2) % 3) / 2);
    for (int m = 0; m < 12; m++) {
      int idx_x = (next_x + (m >> 0 & 1) * 0.6f - 0.3f) - WORLD_SIZE;
      int idx_z = (next_z + (m >> 1 & 1) * 0.6f - 0.3f) - WORLD_SIZE;
      int idx_y = (next_y + ((m >> 2) - 1) * 0.8f + 0.65f) - WORLD_SIZE;
      if ( idx_x < 0 || idx_x >= WORLD_SIZE
        || idx_y < 0 || idx_y >= WORLD_SIZE
        || idx_z < 0 || idx_z >= WORLD_SIZE
        || world[GET_BLOCK_IDX(idx_x, idx_y, idx_z)] > 0) {
        if (dim != 1)
          goto end;
        if (keyboard[' '] > 0 && acc.y > 0.0f) {
          keyboard[' '] = 0;
          acc.y = -JUMP_STRENGH;
          goto end;
        }
        acc.y = 0.0f;
        goto end;
      }
    }
    pos.x = next_x;
    pos.y = next_y;
    pos.z = next_z;
    end:
  }

  if (mouse_left > 0 && lookat_block_idx > 0) {
    world[lookat_block_idx] = 0;
    mouse_left = 0;
  }
  if (mouse_right > 0 && lookat_block_idx > 0) {
    world[lookat_block_idx + lookat_closer_offset] = 1;
    mouse_right = 0;
  }
  for (int m = 0; m < 12; m++) {
    int idx_x = (pos.x + (m >> 0 & 1) * 0.6f - 0.3f) - WORLD_SIZE;
    int idx_y = (pos.y + ((m >> 2) - 1) * 0.8f + 0.65f) - WORLD_SIZE;
    int idx_z = (pos.z + (m >> 1 & 1) * 0.6f - 0.3f) - WORLD_SIZE;
    if ( idx_x >= 0 && idx_x < WORLD_SIZE
      && idx_y >= 0 && idx_y < WORLD_SIZE
      && idx_z >= 0 && idx_z < WORLD_SIZE) {
      world[GET_BLOCK_IDX(idx_x, idx_y, idx_z)] = 0;
    }
  }

  render();
}

void MC_handleKeyboard(int down, int key) {
  if (key < 128)
    keyboard[key] = down;
}

void MC_handleMouse(int down, int left, int right) {
  if (down) {
    mouse_right = right;
    mouse_left = left;
  } else {
    mouse_right = 0;
    mouse_left = 0;
  }
}

void MC_handleMousePos(int x, int y) {
  mouse_x = x;
  mouse_y = y;
}
