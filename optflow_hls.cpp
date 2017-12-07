/**
 * @brief Custom implementation of LK optical flow. O(1) sliding window version. Ref doc for more info
 *
 * @author Pari Kannan (parik@xilinx.com)
 */

#include <stdio.h>
#include <string.h>
#include <ap_int.h>
#include <hls_stream.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include "assert.h"
#include "optflow.h"

// enable to run c-sim
//#define HLS_SIM

// read external array matB and stream. 
// Can be simplified to a single loop with II=1 TODO
void readMatRows (mywide_t *matB, hls::stream <mywide_t>& pixStream)
{
#if 0
  for (int i=0; i < NUM_ITERS; ++i) {

    for (int j=0; j < WORDS_PER_ITER; ++j) {
    #pragma HLS PIPELINE
      mywide_t tmpData = matB [i*WORDS_PER_ITER + j];
      pixStream. write (tmpData);
    }
  }
#endif
  int pix_index = 0;
  for (int i=0; i < MAX_HEIGHT; ++i) {
    for (int j=0; j < MAX_WIDTH/2; ++j) {
    #pragma HLS PIPELINE
      mywide_t tmpData = matB [pix_index++];
      pixStream. write (tmpData);

    }
  }
}

// write rgba stream to external array dst. The "a" is just padding and is
// unused
void writeMatRowsRGBA (hls::stream <rgba_t>& pixStream0, 
                       hls::stream <rgba_t>& pixStream1,
                       rgba2_t *dst)
{
#if 0
  for (int i=0; i < NUM_ITERS; ++i) {
    for (int j=0; j < WORDS_PER_ITER; ++j) {
    #pragma HLS PIPELINE
      rgba2_t tmpData;
      rgba_t d0 = pixStream0. read ();
      rgba_t d1 = pixStream1. read ();
      tmpData.r0 = d0.r; tmpData.g0 = d0.g; tmpData.b0 = d0.b; tmpData.a0 = d0.a;
      tmpData.r1 = d1.r; tmpData.g1 = d1.g; tmpData.b1 = d1.b; tmpData.a1 = d1.a;
      dst [i*WORDS_PER_ITER +j] = tmpData;
    }
  }
#endif
  int pix_index = 0;
  for (int i=0; i < MAX_HEIGHT; ++i) {
    for (int j=0; j < MAX_WIDTH/2; ++j) {
    #pragma HLS PIPELINE
      rgba2_t tmpData;
      rgba_t d0 = pixStream0. read ();
      rgba_t d1 = pixStream1. read ();
      tmpData.r0 = d0.r; tmpData.g0 = d0.g; tmpData.b0 = d0.b; tmpData.a0 = d0.a;
      tmpData.r1 = d1.r; tmpData.g1 = d1.g; tmpData.b1 = d1.b; tmpData.a1 = d1.a;
      dst [pix_index++] = tmpData;
    }
  }
}

// Custom low cost colorizer. Uses RGB to show 4 directions. For
// simple demo purposes only. Real applications would take flow values and do
// other useful analysis on them.
void getPseudoColorInt (pix_t pix, float fx, float fy, rgba_t& rgba)
{
  // normalization factor is key for good visualization. Make this auto-ranging
  // or controllable from the host TODO
  const int normFac = 128/2;

  int y = 127 + (int) (fy * normFac);
  int x = 127 + (int) (fx * normFac);
  if (y>255) y=255;
  if (y<0) y=0;
  if (x>255) x=255;
  if (x<0) x=0;

  rgb_t rgb;
  if (x > 127) {
    if (y < 128) {
      // 1 quad
      rgb.r = x - 127 + (127-y)/2;
      rgb.g = (127 - y)/2;
      rgb.b = 0;
    } else {
      // 4 quad
      rgb.r = x - 127;
      rgb.g = 0;
      rgb.b = y - 127;
    }
  } else {
    if (y < 128) {
      // 2 quad
      rgb.r = (127 - y)/2;
      rgb.g = 127 - x + (127-y)/2;
      rgb.b = 0;
    } else {
      // 3 quad
      rgb.r = 0;
      rgb.g = 128 - x;
      rgb.b = y - 127;
    }
  }

  //rgba.r = pix/2 + rgb.r/2; 
  //rgba.g = pix/2 + rgb.g/2; 
  //rgba.b = pix/2 + rgb.b/2 ;
  rgba.r = rgb.r; 
  rgba.g = rgb.g; 
  rgba.b = rgb.b ;
  rgba.a = 0;
}

// Compute sums for bottom-right and top-right pixel and update the column sums.
// Use column sums to update the integrals. Implements O(1) sliding window.
//
// TODO: 
// 1. Dont need the entire column for img1Win and img2Win. Need only the kernel
// 2. Full line buffer is not needed
void computeSums (hls::stream <mywide_t> img1Col [KMEDP1], 
                  hls::stream <mywide_t> img2Col [KMEDP1], 
                  hls::stream <int>& ixix_out0, 
                  hls::stream <int>& ixiy_out0, 
                  hls::stream <int>& iyiy_out0, 
                  hls::stream <int>& dix_out0, 
                  hls::stream <int>& diy_out0,

                  hls::stream <int>& ixix_out1, 
                  hls::stream <int>& ixiy_out1, 
                  hls::stream <int>& iyiy_out1, 
                  hls::stream <int>& dix_out1, 
                  hls::stream <int>& diy_out1
                  )
                  
{
  pix_t img1Col0 [KMEDP1], img2Col0 [KMEDP1];
  pix_t img1Col1 [KMEDP1], img2Col1 [KMEDP1];
#pragma HLS ARRAY_PARTITION variable=img1Col0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=img2Col0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=img1Col1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=img2Col1 complete dim=0

  static pix_t img1Win [2 * KMEDP1], img2Win [1 * KMEDP1];
#pragma HLS ARRAY_PARTITION variable=img1Win complete dim=0
#pragma HLS ARRAY_PARTITION variable=img2Win complete dim=0
  //static pix_t img1Win1 [2 * KMEDP1], img2Win1 [1 * KMEDP1];
//#pragma HLS ARRAY_PARTITION variable=img1Win1 complete dim=0
//#pragma HLS ARRAY_PARTITION variable=img2Win1 complete dim=0

  static int ixix=0, ixiy=0, iyiy=0, dix=0, diy=0;

  // column sums:
  // need left-shift. Array-Part leads to FF with big Muxes. Try to do with
  // classic array and pointer. Need current and current-KMED ptrs
  // For II=1 pipelining, need two read and 1 write ports. Simulating it with
  // two arrays that have their write ports tied together.
  // TODO need only MAX_WODTH/2. Have to adjust zIdx and nIdx as well
  static int csIxixO [MAX_WIDTH], csIxiyO [MAX_WIDTH], csIyiyO [MAX_WIDTH], csDixO [MAX_WIDTH], csDiyO [MAX_WIDTH];
  static int csIxixE [MAX_WIDTH], csIxiyE [MAX_WIDTH], csIyiyE [MAX_WIDTH], csDixE [MAX_WIDTH], csDiyE [MAX_WIDTH];

  static int cbIxixO [MAX_WIDTH], cbIxiyO [MAX_WIDTH], cbIyiyO [MAX_WIDTH], cbDixO [MAX_WIDTH], cbDiyO [MAX_WIDTH];
  static int cbIxixE [MAX_WIDTH], cbIxiyE [MAX_WIDTH], cbIyiyE [MAX_WIDTH], cbDixE [MAX_WIDTH], cbDiyE [MAX_WIDTH];

  int zIdx= - (KMED-2);   // odd
  int zIdx1 = zIdx + 1;   // even

  int nIdx = zIdx + KMED-2; // even (0)
  int nIdx1 = nIdx + 1;     // odd

#pragma HLS RESOURCE variable=csIxixO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csIxiyO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csIyiyO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csDixO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csDiyO core=RAM_2P_BRAM
#pragma HLS DEPENDENCE variable=csIxixO inter WAR false
#pragma HLS DEPENDENCE variable=csIxiyO inter WAR false
#pragma HLS DEPENDENCE variable=csIyiyO inter WAR false
#pragma HLS DEPENDENCE variable=csDixO  inter WAR false
#pragma HLS DEPENDENCE variable=csDiyO  inter WAR false

#pragma HLS RESOURCE variable=csIxixE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csIxiyE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csIyiyE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csDixE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=csDiyE core=RAM_2P_BRAM
#pragma HLS DEPENDENCE variable=csIxixE inter WAR false
#pragma HLS DEPENDENCE variable=csIxiyE inter WAR false
#pragma HLS DEPENDENCE variable=csIyiyE inter WAR false
#pragma HLS DEPENDENCE variable=csDixE  inter WAR false
#pragma HLS DEPENDENCE variable=csDiyE  inter WAR false


#pragma HLS RESOURCE variable=cbIxixO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbIxiyO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbIyiyO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbDixO core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbDiyO core=RAM_2P_BRAM
#pragma HLS DEPENDENCE variable=cbIxixO inter WAR false
#pragma HLS DEPENDENCE variable=cbIxiyO inter WAR false
#pragma HLS DEPENDENCE variable=cbIyiyO inter WAR false
#pragma HLS DEPENDENCE variable=cbDixO  inter WAR false
#pragma HLS DEPENDENCE variable=cbDiyO  inter WAR false

#pragma HLS RESOURCE variable=cbIxixE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbIxiyE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbIyiyE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbDixE core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=cbDiyE core=RAM_2P_BRAM
#pragma HLS DEPENDENCE variable=cbIxixE inter WAR false
#pragma HLS DEPENDENCE variable=cbIxiyE inter WAR false
#pragma HLS DEPENDENCE variable=cbIyiyE inter WAR false
#pragma HLS DEPENDENCE variable=cbDixE  inter WAR false
#pragma HLS DEPENDENCE variable=cbDiyE  inter WAR false

  int csIxixR0, csIxiyR0, csIyiyR0, csDixR0, csDiyR0;
  int csIxixR1, csIxiyR1, csIyiyR1, csDixR1, csDiyR1;

  for (int r = 0; r < MAX_HEIGHT; r++) {
    for (int c = 0; c < MAX_WIDTH/2; c++) {
      #pragma HLS PIPELINE

      int csIxixL0 = 0, csIxiyL0 = 0, csIyiyL0 = 0, csDixL0  = 0, csDiyL0  = 0;
      int csIxixL1 = 0, csIxiyL1 = 0, csIyiyL1 = 0, csDixL1  = 0, csDiyL1  = 0;

      if (zIdx >= 0) {
        csIxixL0 = csIxixO [zIdx];
        csIxiyL0 = csIxiyO [zIdx];
        csIyiyL0 = csIyiyO [zIdx];
        csDixL0  = csDixO [zIdx];
        csDiyL0  = csDiyO [zIdx];
      }
      if (zIdx1 >= 0) {
        csIxixL1 = csIxixE [zIdx1];
        csIxiyL1 = csIxiyE [zIdx1];
        csIyiyL1 = csIyiyE [zIdx1];
        csDixL1  = csDixE [zIdx1];
        csDiyL1  = csDiyE [zIdx1];
      }

      for (int wr=0; wr<KMEDP1; ++wr) {
        mywide_t tmp1 = img1Col [wr]. read ();
        img1Col0[wr] = tmp1. data [0];
        img1Col1[wr] = tmp1. data [1];

        mywide_t tmp2 = img2Col [wr]. read ();
        img2Col0[wr] = tmp2. data [0];
        img2Col1[wr] = tmp2. data [1];
      }

      // p(x+1,y) and p(x-1,y)
      int wrt=1;
      int cIxTopR0 = (img1Col0 [wrt] - img1Win [wrt*2 + 2-2]) /2 ;
      // p(x,y+1) and p(x,y-1)
      int cIyTopR0 = (img1Win [ (wrt+1)*2 + 2-1] - img1Win [ (wrt-1)*2 + 2-1])  /2;
      // p1(x,y) and p2(x,y)
      int delTopR0 = img1Win [wrt*2 + 2-1] - img2Win [wrt*1 + 1-1];

      int wrb = KMED-1;
      int cIxBotR0 = (img1Col0 [wrb] - img1Win [wrb*2 + 2-2]) /2 ;
      int cIyBotR0 = (img1Win [ (wrb+1)*2 + 2-1] - img1Win [ (wrb-1)*2 + 2-1]) /2;
      int delBotR0 = img1Win [wrb*2 + 2-1] - img2Win [wrb*1 + 1-1];
      if (0 && r < KMED) {
        cIxTopR0 = 0; cIyTopR0 = 0; delTopR0 = 0;
      }
      
      // p(x+1,y) and p(x-1,y)
      wrt=1;
      int cIxTopR1 = (img1Col1 [wrt] - img1Win [wrt*2 + 2-1]) /2 ;
      // p(x,y+1) and p(x,y-1)
      int cIyTopR1 = (img1Col0 [wrt + 1] - img1Col0 [wrt - 1]) /2; 
      // p1(x,y) and p2(x,y)
      int delTopR1 = (img1Col0 [wrt] - img2Col0 [wrt]);

      wrb = KMED-1;
      int cIxBotR1 = (img1Col1 [wrb] - img1Win [wrb*2 + 2-1]) /2 ;
      int cIyBotR1 = (img1Col0 [wrb + 1] - img1Col0 [wrb - 1]) /2; 
      int delBotR1 = (img1Col0 [wrb] - img2Col0 [wrb]);

      csIxixR0 = cbIxixE [nIdx] + cIxBotR0 * cIxBotR0 - cIxTopR0 * cIxTopR0;
      csIxiyR0 = cbIxiyE [nIdx] + cIxBotR0 * cIyBotR0 - cIxTopR0 * cIyTopR0;
      csIyiyR0 = cbIyiyE [nIdx] + cIyBotR0 * cIyBotR0 - cIyTopR0 * cIyTopR0;
      csDixR0  = cbDixE [nIdx]  + delBotR0 * cIxBotR0 - delTopR0 * cIxTopR0;
      csDiyR0  = cbDiyE [nIdx]  + delBotR0 * cIyBotR0 - delTopR0 * cIyTopR0;

      csIxixR1 = cbIxixO [nIdx1] + cIxBotR1 * cIxBotR1 - cIxTopR1 * cIxTopR1;
      csIxiyR1 = cbIxiyO [nIdx1] + cIxBotR1 * cIyBotR1 - cIxTopR1 * cIyTopR1;
      csIyiyR1 = cbIyiyO [nIdx1] + cIyBotR1 * cIyBotR1 - cIyTopR1 * cIyTopR1;
      csDixR1  = cbDixO [nIdx1]  + delBotR1 * cIxBotR1 - delTopR1 * cIxTopR1;
      csDiyR1  = cbDiyO [nIdx1]  + delBotR1 * cIyBotR1 - delTopR1 * cIyTopR1;

      int tmpixix0 = (csIxixR0 - csIxixL0);
      int tmpixix1 = (csIxixR0 - csIxixL0) + (csIxixR1 - csIxixL1);
      int tmpixiy0 = (csIxiyR0 - csIxiyL0);
      int tmpixiy1 = (csIxiyR0 - csIxiyL0) + (csIxiyR1 - csIxiyL1);
      int tmpiyiy0 = (csIyiyR0 - csIyiyL0);
      int tmpiyiy1 = (csIyiyR0 - csIyiyL0) + (csIyiyR1 - csIyiyL1);
      int tmpdix0  = (csDixR0 - csDixL0);
      int tmpdix1  = (csDixR0 - csDixL0) + (csDixR1 - csDixL1);
      int tmpdiy0  = (csDiyR0 - csDiyL0);
      int tmpdiy1  = (csDiyR0 - csDiyL0) + (csDiyR1 - csDiyL1);

      //ixix += (csIxixR0 - csIxixL0);
      //ixiy += (csIxiyR0 - csIxiyL0);
      //iyiy += (csIyiyR0 - csIyiyL0);
      //dix += (csDixR0 - csDixL0);
      //diy += (csDiyR0 - csDiyL0);

      //ixix_out0. write (ixix);
      //ixiy_out0. write (ixiy);
      //iyiy_out0. write (iyiy);
      //dix_out0. write (dix);
      //diy_out0. write (diy);
      ixix_out0. write (ixix + tmpixix0);
      ixiy_out0. write (ixiy + tmpixiy0);
      iyiy_out0. write (iyiy + tmpiyiy0);
      dix_out0. write (dix + tmpdix0);
      diy_out0. write (diy + tmpdiy0);

      // now compute the second pixel
      //ixix += (csIxixR1 - csIxixL1);
      //ixiy += (csIxiyR1 - csIxiyL1);
      //iyiy += (csIyiyR1 - csIyiyL1);
      //dix += (csDixR1 - csDixL1);
      //diy += (csDiyR1 - csDiyL1);
      ixix += tmpixix1;
      ixiy += tmpixiy1;
      iyiy += tmpiyiy1;
      dix += tmpdix1;
      diy += tmpdiy1;

      ixix_out1. write (ixix);
      ixiy_out1. write (ixiy);
      iyiy_out1. write (iyiy);
      dix_out1. write (dix);
      diy_out1. write (diy);

      for (int i = 0; i < KMEDP1; i++) {
        img1Win [i * 2] = img1Col0 [i];
        img1Win [i * 2 + 1] = img1Col1 [i];
        img2Win [i] = img2Col1 [i];
      }

      cbIxixE [nIdx] = csIxixR0;
      cbIxiyE [nIdx] = csIxiyR0;
      cbIyiyE [nIdx] = csIyiyR0;
      cbDixE  [nIdx] = csDixR0;
      cbDiyE  [nIdx] = csDiyR0;

      csIxixE [nIdx] = csIxixR0;
      csIxiyE [nIdx] = csIxiyR0;
      csIyiyE [nIdx] = csIyiyR0;
      csDixE  [nIdx] = csDixR0;
      csDiyE  [nIdx] = csDiyR0;

      cbIxixO [nIdx1] = csIxixR1;
      cbIxiyO [nIdx1] = csIxiyR1;
      cbIyiyO [nIdx1] = csIyiyR1;
      cbDixO  [nIdx1] = csDixR1;
      cbDiyO  [nIdx1] = csDiyR1;

      csIxixO [nIdx1] = csIxixR1;
      csIxiyO [nIdx1] = csIxiyR1;
      csIyiyO [nIdx1] = csIyiyR1;
      csDixO  [nIdx1] = csDixR1;
      csDiyO  [nIdx1] = csDiyR1;

      // zIdx is always odd, zIdx1 is even
      // nIdx is always even, nIdx1 is odd
      zIdx += 2;
      if (zIdx >= MAX_WIDTH) zIdx = 1;
      zIdx1 += 2;
      if (zIdx1 == MAX_WIDTH) zIdx1 = 0;

      nIdx += 2;
      if (nIdx == MAX_WIDTH) nIdx = 0;
      nIdx1 += 2;
      if (nIdx1 >= MAX_WIDTH) nIdx1 = 1;
    }
  }

  // Cleanup. If kernel is called multiple times with different inputs, not
  // cleaning these vars would pollute the subsequent frames.
  // TODO zero in the line buffer instead, for r < KMED
  for (int r = 0; r < KMEDP1; r++) {
    #pragma HLS PIPELINE
    img1Win [r] = 0; img1Win [r+KMEDP1] = 0; img2Win [r] = 0;
    img1Col0 [r] =0; img2Col0 [r] =0;
    img1Col1 [r] =0; img2Col1 [r] =0;
  }
  for (int r=0; r < MAX_WIDTH; ++r) {
    #pragma HLS PIPELINE
    csIxixO [r] = 0; csIxiyO [r] = 0; csIyiyO [r] = 0; csDixO [r] = 0; csDiyO [r] = 0;
    cbIxixO [r] = 0; cbIxiyO [r] = 0; cbIyiyO [r] = 0; cbDixO [r] = 0; cbDiyO [r] = 0;

    csIxixE [r] = 0; csIxiyE [r] = 0; csIyiyE [r] = 0; csDixE [r] = 0; csDiyE [r] = 0;
    cbIxixE [r] = 0; cbIxiyE [r] = 0; cbIyiyE [r] = 0; cbDixE [r] = 0; cbDiyE [r] = 0;
  }
  ixix=0; ixiy=0; iyiy=0; dix=0; diy=0;
}

// consume the integrals and compute flow vectors
void computeFlow (hls::stream <int>& ixix, 
                  hls::stream <int>& ixiy, 
                  hls::stream <int>& iyiy, 
                  hls::stream <int>& dix, 
                  hls::stream <int>& diy, 
                  hls::stream <float>& fx_out, 
                  hls::stream <float>& fy_out)
{
  for (int r = 0; r < MAX_HEIGHT; r++) {
    for (int c = 0; c < MAX_WIDTH/2; c++) {
      #pragma HLS PIPELINE
      int ixix_ = ixix. read ();
      int ixiy_ = ixiy. read ();
      int iyiy_ = iyiy. read ();
      int dix_  = dix. read ();
      int diy_  = diy. read ();
      float fx_=0, fy_=0;

      // matrix inv
      float det = (float)ixix_ * iyiy_ - (float)ixiy_ * ixiy_;
      if (det <= 1.0f) {
        fx_ = 0.0;
        fy_ = 0.0;
      } else {
        // res est: (dsp,ff,lut)
        // fdiv (0,748,800), fmul (3,143,139), fadd (2,306,246), fsub (2,306,246)
        // sitofp (0,229,365), fcmp (0,66,72), imul(1,0,0) (in cs)
        //float detInv = 1.0/det;
        float i00 = (float)iyiy_ / det;
        float i01 = (float) (-ixiy_) / det;
        float i10 = (float) (-ixiy_) / det;
        float i11 = (float)ixix_ / det;

        fx_ = i00 * dix_ + i01 * diy_;
        fy_ = i10 * dix_ + i11 * diy_;
      }
      fx_out. write (fx_);
      fy_out. write (fy_);
    }
  }
}

// convert flow values to visualizable pixel. For simple demo purposes only
void getOutPix (hls::stream <float>& fx, 
                hls::stream <float>& fy, 
                hls::stream <rgba_t>& out_pix)
{
  for (int r = 0; r < MAX_HEIGHT; r++) {
    for (int c = 0; c < MAX_WIDTH/2; c++) {
      #pragma HLS PIPELINE
      float fx_ = fx. read ();
      float fy_ = fy. read ();

      pix_t p_  = 0;
      rgba_t out_pix_;
      getPseudoColorInt (p_, fx_, fy_, out_pix_);

      out_pix. write (out_pix_);
    }
  }
}

// line buffer for both input images. Can be split to a fn that models a single
// linebuffer
void lbWrapper (hls::stream <mywide_t>& f0Stream, 
                hls::stream <mywide_t>& f1Stream, 
                hls::stream <mywide_t> img1Col[KMEDP1], 
                hls::stream <mywide_t> img2Col[KMEDP1])
{
  static mywide_t lb1 [KMEDP1][MAX_WIDTH/2], lb2 [KMEDP1][MAX_WIDTH/2];
#pragma HLS ARRAY_PARTITION variable=lb1 complete dim=1
#pragma HLS ARRAY_PARTITION variable=lb2 complete dim=1

  for (int r = 0; r < MAX_HEIGHT; r++) {
    for (int c = 0; c < MAX_WIDTH/2; c++) {
      #pragma HLS PIPELINE

      // shift up both linebuffers at col=c
      for (int i = 0; i < KMEDP1 - 1; i++) {
        lb1 [i][c] = lb1 [i + 1][c];
        img1Col [i]. write (lb1 [i][c]);

        lb2 [i][c] = lb2 [i+1][c];
        img2Col [i]. write (lb2 [i][c]);
      }

      // read in the new pixels at col=c and row=bottom_of_lb
      mywide_t pix0 = f0Stream. read ();
      lb1 [KMEDP1 - 1][c] = pix0;
      img1Col [KMEDP1 - 1]. write (pix0);

      mywide_t pix1 = f1Stream. read ();
      lb2 [KMEDP1 -1][c] = pix1;
      img2Col [KMEDP1 - 1]. write (pix1);
    }
  }


  // cleanup
  mywide_t tmpClr;
  tmpClr. data [0] = 0;
  tmpClr. data [1] = 0;
  for (int r = 0; r < KMEDP1; r++) {
    for (int c = 0; c < MAX_WIDTH/2; c++) {
      #pragma HLS PIPELINE
      lb1 [r][c] = tmpClr;
      lb2 [r][c] = tmpClr;
    }
  }
}

// top level wrapper to avoid dataflow problems
void flowWrap (mywide_t frame0[NUM_WORDS], mywide_t frame1[NUM_WORDS], rgba2_t framef[NUM_WORDS])
{
#pragma HLS data_pack variable=frame0
#pragma HLS data_pack variable=frame1
#pragma HLS data_pack variable=framef

#pragma HLS DATAFLOW

  // ddr <-> kernel streams. Stream depths are probably too large and can be
  // trimmed
  hls::stream <mywide_t> f0Stream, f1Stream;
#pragma HLS data_pack variable=f0Stream
#pragma HLS data_pack variable=f1Stream
#pragma HLS STREAM variable=f0Stream depth=16
#pragma HLS STREAM variable=f1Stream depth=16

  hls::stream <rgba_t> ff0Stream, ff1Stream;
#pragma HLS data_pack variable=ff0Stream
#pragma HLS data_pack variable=ff1Stream
#pragma HLS STREAM variable=ff0Stream depth=16
#pragma HLS STREAM variable=ff1Stream depth=16

  hls::stream <mywide_t> img1Col [KMEDP1], img2Col [KMEDP1];
#pragma HLS data_pack variable=img1Col
#pragma HLS data_pack variable=img2Col
#pragma HLS STREAM variable=img1Col  depth=16
#pragma HLS STREAM variable=img2Col  depth=16
#pragma HLS ARRAY_PARTITION variable=img1Col complete dim=0
#pragma HLS ARRAY_PARTITION variable=img2Col complete dim=0

  hls::stream <int> ixix0, ixiy0, iyiy0, dix0, diy0;
  hls::stream <float> fx0, fy0;
#pragma HLS STREAM variable=ixix0 depth=16
#pragma HLS STREAM variable=ixiy0 depth=16
#pragma HLS STREAM variable=iyiy0 depth=16
#pragma HLS STREAM variable=dix0 depth=16
#pragma HLS STREAM variable=diy0 depth=16
#pragma HLS STREAM variable=fx0  depth=16
#pragma HLS STREAM variable=fy0  depth=16

  hls::stream <int> ixix1, ixiy1, iyiy1, dix1, diy1;
  hls::stream <float> fx1, fy1;
#pragma HLS STREAM variable=ixix1 depth=16
#pragma HLS STREAM variable=ixiy1 depth=16
#pragma HLS STREAM variable=iyiy1 depth=16
#pragma HLS STREAM variable=dix1 depth=16
#pragma HLS STREAM variable=diy1 depth=16
#pragma HLS STREAM variable=fx1  depth=16
#pragma HLS STREAM variable=fy1  depth=16

  readMatRows (frame0, f0Stream);
  readMatRows (frame1, f1Stream);

  lbWrapper (f0Stream, f1Stream, img1Col, img2Col);
  computeSums (img1Col, img2Col, 
               ixix0, ixiy0, iyiy0, dix0, diy0, 
               ixix1, ixiy1, iyiy1, dix1, diy1);

  computeFlow (ixix0, ixiy0, iyiy0, dix0, diy0, fx0, fy0);
  computeFlow (ixix1, ixiy1, iyiy1, dix1, diy1, fx1, fy1);

  getOutPix (fx0, fy0, ff0Stream);
  getOutPix (fx1, fy1, ff1Stream);

  writeMatRowsRGBA (ff0Stream,  ff1Stream, framef);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#if 0
// external interface to the kernel. 
//  frame0 - First input frame (grayscale 1 byte per pixel)
//  frame1 - Second input frame (grayscale 1 byte per pixel)
//  framef - Output frame with flows visualized. 3 bytes per pixel + 1 byte padding 
#ifdef HLS_SIM 
void fpga_optflow (mywide_t frame0 [NUM_WORDS], mywide_t frame1 [NUM_WORDS], rgba2_t framef [NUM_WORDS]) 
#else
extern "C" {
void fpga_optflow (mywide_t frame0 [NUM_WORDS], mywide_t frame1 [NUM_WORDS], rgba2_t framef [NUM_WORDS]) 
#endif
{

#pragma HLS data_pack variable=frame0
#pragma HLS data_pack variable=frame1
#pragma HLS data_pack variable=framef


#pragma HLS INTERFACE m_axi port=frame0 offset=slave bundle=gmem num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS INTERFACE m_axi port=frame1 offset=slave bundle=gmem1 num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS INTERFACE m_axi port=framef offset=slave bundle=gmem2 num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=64 max_read_burst_length=64

#pragma HLS INTERFACE s_axilite port=frame0 bundle=control
#pragma HLS INTERFACE s_axilite port=frame1 bundle=control
#pragma HLS INTERFACE s_axilite port=framef bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

#pragma HLS inline region off

  flowWrap (frame0, frame1, framef);

  return;

}

#ifndef HLS_SIM
}
#endif

#endif //0
