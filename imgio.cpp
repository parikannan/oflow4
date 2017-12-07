//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#include "imgio.h"
#include "sds_lib.h"

#include <assert.h>

namespace ImgIoUtil {

bool savePpm (const std::string& fnme, char *p_frame, int width, int height)
{
  std::ofstream ofs (fnme. c_str ());
  if (!ofs) {
    std::cout << "Failed to write file " << fnme << std::endl;
    return false;
  } 
    
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (int h=0; h<height; ++h) {
    for (int w=0; w<width; ++w) { 
      char c = p_frame [h*width + w];
      ofs << c << c << c;
    }
  }
  ofs.close ();
  return true;
}

void toRGB (const yuv_t& yuv, rgb_t& rgb)
{
  rgb.r = (pix_t) (yuv.y + 1.4075 * (yuv.v - 128));
  rgb.g = (pix_t) (yuv.y - 0.3455 * (yuv.u - 128) - 0.7169 * (yuv.v - 128));
  rgb.b = (pix_t) (yuv.y + 1.7790 * (yuv.u - 128));
} 

void toYuv (const rgb_t& rgb, yuv_t& yuv)
{
  yuv.y = (pix_t) (rgb.r * 0.299000  + rgb.g * 0.587000  + rgb.b * 0.114000);
  yuv.u = 128 + (pix_t) (rgb.r * -0.168736 + rgb.g * -0.331264 + rgb.b * 0.500000);
  yuv.v = 128 + (pix_t) (rgb.r * 0.500000  + rgb.g * -0.418688 + rgb.b * -.081312);
}

void toYuv (const rgba_t& rgba, yuv_t& yuv)
{
  rgb_t rgb;
  rgb.r = rgba.r;
  rgb.g = rgba.g;
  rgb.b = rgba.b;
  toYuv (rgb, yuv);
}

// convert yuv to rgb and write out standard ppm
bool savePpm (const std::string& fnme, yuv_t *p_frame, int width, int height)
{
  std::ofstream ofs (fnme. c_str ());
  if (!ofs) {
    std::cout << "Failed to write file " << fnme << std::endl;
    return false;
  } 
    
  rgb_t rgb;

  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (int h=0; h<height; ++h) {
    for (int w=0; w<width; ++w) { 
      yuv_t p = p_frame [h*width + w];
      toRGB (p, rgb);
      ofs << rgb.r << rgb.g << rgb.b;
    }
  }
  ofs.close ();
  return true;
}

bool savePpm (const std::string& fnme, rgba_t *p_frame, int width, int height)
{
  std::ofstream ofs (fnme. c_str ());
  if (!ofs) {
    std::cout << "Failed to write file " << fnme << std::endl;
    return false;
  } 
    
  rgba_t rgba;

  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (int h=0; h<height; ++h) {
    for (int w=0; w<width; ++w) { 
      rgba = p_frame [h*width + w];
      ofs << rgba.r << rgba.g << rgba.b;
    }
  }
  ofs.close ();
  return true;
}

rgb_t* readPpm (const std::string& fnme, int w, int h)
{
  std::ifstream ifs (fnme. c_str ());
  if (!ifs) {
    std::cout << "Failed to open file " << fnme << std::endl;
    return NULL;
  } 
    
  std::string prefix;
  ifs >> prefix;
  if (prefix != "P6") {
    std::cout << "Invalid ppm file " << fnme << std::endl;
    return NULL;
  }
  int w_, h_, maxLvl, padRows=0;
  ifs >> w_;
  ifs >> h_;
  ifs >> maxLvl; 
  if (h > h_) padRows = h-h_;
  std::cout << "ppm header w=" << w_ << " h=" << h_ << " padRows=" << padRows << " maxLvl=" << maxLvl << std::endl;
  if (w_ != w || h_+padRows != h) {
    std::cout << "Image dim mismatch" << std::endl;
    return NULL;
  }

  // eat newline after maxlevel
  char tmp;
  ifs >> std::noskipws >> tmp;
  
  rgb_t *p_frame =  new rgb_t [w*h];

  for (int i=0; i< w*h_; ++i) {
    rgb_t rgb; 
    ifs >> std::noskipws >> rgb.r;
    ifs >> std::noskipws >> rgb.g;
    ifs >> std::noskipws >> rgb.b;
    p_frame [i] = rgb;
  }

  for (int i=0; i< w*padRows; ++i) {
    rgb_t rgb = {0, 0, 0};
    p_frame [w*h_ + i] = rgb;
  }

  ifs. close ();
  return p_frame;
}

// read ppm image, extract y into p_frame
pix_t* readPpmY (const std::string& fnme, int w, int h)
{
  rgb_t *p_rgb = readPpm (fnme, w, h);
  if (!p_rgb) {
    std::cout << "readPpmY failed to parse " << fnme << std::endl;
    return NULL;
  }

  //pix_t *p_y = new pix_t [w*h];
  pix_t *p_y = (pix_t*) sds_alloc_non_cacheable (w*h*sizeof(pix_t));

  for (int i=0; i<w*h; ++i) {
    rgb_t rgb = p_rgb [i];
    yuv_t yuv;
    toYuv (rgb, yuv);
    p_y [i] = yuv.y;
  }
  delete [] p_rgb;

  return p_y;
}

// yuv422p nPixels of y, nPixels/2 of u and nPixels/2 of v
void writeYuv444p (std::ofstream& ofs, rgba_t *p_rgba, int w, int h)
{
  int nPixels = w * h;
  char *p_y = new char [nPixels];
  char *p_u = new char [nPixels];
  char *p_v = new char [nPixels];

  for (int i=0; i<nPixels; ++i) {
    // src is in rgb.
    rgba_t rgba = p_rgba [i];
    yuv_t yuv;
    toYuv (rgba, yuv);
    p_y [i] = yuv.y;
    p_u [i] = yuv.u;
    p_v [i] = yuv.v;
  }
  ofs. write (p_y, nPixels);
  ofs. write (p_u, nPixels);
  ofs. write (p_v, nPixels);

  delete [] p_y;
  delete [] p_u;
  delete [] p_v;
}

// p_in dims: in_w * in_h
// p_out should be in_w*wscale * in_h*hscale + padRows*in_w*wscale
bool getScaledFrame (char *p_in, char *p_out, int in_w, int in_h, int scale, int padRows)
{
  int out_w = in_w * scale;
  int out_h = in_h * scale + padRows;
  char padPix = 0; 
  
  for (int pad=0; pad<padRows/2; ++pad)
    for (int w=0; w< out_w; ++w) {
      *p_out = padPix;
      ++p_out; 
    }
    
  for (int h=0; h < in_h; ++h) {
    for (int sr=0; sr < scale; ++sr) {
    
      for (int w=0; w < in_w; ++w) {

        char c = p_in [h*in_w + w];
        for (int sc=0; sc<scale; ++sc) {
          *p_out = c;
          ++p_out;
        }

      }
    }
  }

  for (int pad=0; pad < padRows-padRows/2; ++pad)
    for (int w=0; w < out_w; ++w) {
      *p_out = padPix;
      ++p_out;
    }

  return true;
}

bool read_image_raw (const char *outf, pix_t *image1, int width, int height) {
  std::cout << "read_image_raw " << outf << std::endl;
  FILE *input = fopen (outf, "r"); 
  if (input == NULL) {
    std::cout << "Couldn't open " << outf << " for reading!" << std::endl;
    return false;
  }
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      fscanf (input, "%hhu ", &(image1[j*width+i]));
    }
    fscanf (input, "\n");
  }
  fclose (input);
  return true;
} 

mywide_t* getWide (pix_t *img, int width, int height)
{
  int nPkd = width * height / WORD_SZ;
  assert (nPkd * WORD_SZ == width*height);
  mywide_t *p = new mywide_t [nPkd];
  for (int i=0; i<nPkd; ++i) {
    for (int w=0; w<WORD_SZ; ++w) {
      p [i]. data [w] = img [i*WORD_SZ +w];
    }
  }
  return p;
}

pix_t* getNative (mywide_t *pkd, int width, int height)
{
  int nPkd = width * height / WORD_SZ;
  assert (nPkd * WORD_SZ == width*height);
  pix_t *p = new pix_t [width * height];
  for (int i=0; i < nPkd; ++i) {
    for (int w=0; w < WORD_SZ; ++w) {
      p [i*WORD_SZ + w] = pkd [i]. data [w];
    }
  }
  return p;
}

} // namespace

