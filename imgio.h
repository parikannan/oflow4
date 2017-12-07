//----------------------------------------------------------------------------
// Utils for Image and Clip I/O
//----------------------------------------------------------------------------

#ifndef IMGIO_H_
#define IMGIO_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include "optflow.h"

namespace ImgIoUtil {


// p_frame is frayscale byte-array. 1 byte per pix
bool savePpm (const std::string& fnme, char *p_frame, int width, int height);

// read ppm image, extract y into p_frame. 1 byte per pix
pix_t* readPpmY (const std::string& fnme, int w, int h);

// convert yuv to rgb and write out standard ppm
bool savePpm (const std::string& fnme, yuv_t *p_frame, int width, int height);

// write rgba to std ppm. drop "a" channel
bool savePpm (const std::string& fnme, rgba_t *p_frame, int width, int height);

rgb_t* readPpm (const std::string& fnme, int w, int h);

void toRGB (const yuv_t& yuv, rgb_t& rgb);

void toYuv (const rgb_t& rgb, yuv_t& yuv);

// yuv444 planar nPixels of y, u and v
void writeYuv444p (std::ofstream& ofs, rgba_t *p_rgba, int w, int h);

// Basic scaling with padding. Does not handle all situations.
// p_in dims: in_w * in_h
// p_out should be in_w*wscale * in_h*hscale + padRows*in_w*wscale
bool getScaledFrame (char *p_in, char *p_out, int in_w, int in_h, int scale, int padRows);

bool read_image_raw (const char *outf, pix_t *image1, int width, int height);

mywide_t* getWide (pix_t *img, int width, int height);
pix_t* getNative (mywide_t *pkd, int width, int height);

};


#endif
