#ifndef OPTFLOW_H
#define OPTFLOW_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef unsigned char pix_t;

// size of the input images
#define MAX_HEIGHT 2160
#define MAX_WIDTH  3840
//#define MAX_HEIGHT 800
//#define MAX_WIDTH  1280

#define NUM_PIXELS (MAX_WIDTH * MAX_HEIGHT)

// size of the sliding window. This should be an odd number >=3 
// make it >= 11 to prevent WAR hazards
#define KMED 53

#define KMEDP1 (KMED+1)

//----------------------------------------------------------------------------
// memory access. These can be greatly simplified. Currently r/w 1 byte per
// cycle.
//----------------------------------------------------------------------------
#define WORD_SZ 2
#define WORDS_PER_ITER 4

#define MAT_ROWS MAX_HEIGHT
#define MAT_COLS MAX_WIDTH
#define BLOCK_SZ MAT_ROWS * MAT_COLS
#define NUM_WORDS BLOCK_SZ / WORD_SZ

#define NUM_ITERS (NUM_WORDS)/WORDS_PER_ITER

typedef struct __wide{
      pix_t data [WORD_SZ];
} mywide_t;

typedef struct __yuv{
      pix_t y, u, v;
} yuv_t;

typedef struct __rgb{
      pix_t r, g, b;
} rgb_t;

// kernel returns this type. Packed structs on axi need to be powers-of-2.
typedef struct __rgba{
      pix_t r, g, b;
      pix_t a;    // can be unused         
} rgba_t;

typedef struct __wide_rgba {
  rgba_t data [WORD_SZ];
} mywide_rgba_t;

typedef struct __rgba2{
      pix_t r0, g0, b0, a0;
      pix_t r1, g1, b1, a1;
} rgba2_t;

typedef struct __hsv{
      pix_t h, s, v;
} hsv_t;


//void flowWrap (mywide_t *frame0, mywide_t *frame1, rgba2_t *framef);

//#pragma SDS data data_mover(frame0:AXIDMA_SIMPLE,frame1:AXIDMA_SIMPLE,framef:AXIDMA_SIMPLE)
#pragma SDS data mem_attribute(frame0:NON_CACHEABLE|PHYSICAL_CONTIGUOUS)
#pragma SDS data mem_attribute(frame1:NON_CACHEABLE|PHYSICAL_CONTIGUOUS)
#pragma SDS data mem_attribute(framef:NON_CACHEABLE|PHYSICAL_CONTIGUOUS)
#pragma SDS data access_pattern(frame0:SEQUENTIAL)
#pragma SDS data access_pattern(frame1:SEQUENTIAL)
#pragma SDS data access_pattern(framef:SEQUENTIAL)
#pragma SDS data zero_copy(frame0[0:NUM_WORDS])
#pragma SDS data zero_copy(frame1[0:NUM_WORDS])
#pragma SDS data zero_copy(framef[0:NUM_WORDS])
void flowWrap (mywide_t frame0[NUM_WORDS], mywide_t frame1[NUM_WORDS], rgba2_t framef[NUM_WORDS]);

#endif

