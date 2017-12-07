#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <assert.h>
#include <sstream>
#include <fstream>

// for img/clip IO
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <CL/opencl.h>

#include "optflow.h"
#include "imgio.h"

//#ifdef __SDSOC
#include "sds_lib.h"
#define TIME_STAMP_INIT  unsigned int clock_start, clock_end;  clock_start = sds_clock_counter();
#define TIME_STAMP  { clock_end = sds_clock_counter(); printf("elapsed time %lu \n", clock_end-clock_start); clock_start = sds_clock_counter();  }
//#endif

using namespace ImgIoUtil;

//extern void flowWrap (mywide_t *frame0, mywide_t *frame1, rgba2_t *framef);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void fpga_optflow (mywide_t *frame0, mywide_t *frame1, rgba2_t *out)
{
  flowWrap (frame0, frame1, out);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


#if 0
int runClip (int argc, char** argv)
{
  if (argc != 7) {
    std::cout << "Usage:\n\ttest-cl.exe inclip.yuv outclip.yuv width height begFrame endFrame\n\tOut clip dims: " 
              << MAX_WIDTH << "x" << MAX_HEIGHT << "\n" << std::endl;
    return 1;
  }

  std::string inFnme (argv[1]), outFnme (argv[2]);
  int in_w = std::atoi (argv [3]);
  int in_h = std::atoi (argv [4]);
  int begFrame = std::atoi (argv [5]);
  int endFrame = std::atoi (argv [6]);

  std::cout << "runClip " << inFnme << " -> " << outFnme << " Frames: " << begFrame << " -> " << endFrame << std::endl;

  int dst_h=MAX_HEIGHT, dst_w=MAX_WIDTH;
  int dst_pixPerFrame = dst_w * dst_h;
  int dst_pkd = dst_w * dst_h / WORD_SZ;
  assert (dst_pkd * WORD_SZ == dst_w*dst_h);

  int scale = 1;
  int padRows = dst_h - in_h * scale;
  if (padRows < 0) padRows = 0;

  int in_pixPerFrame = in_w * in_h * 3/2;
  int in_yPerFrame  = in_w * in_h;
  int in_uvPerFrame = in_w * in_h/2;

  char *p_yBuf = new char [in_yPerFrame];
  char *p_uvBuf = new char [in_uvPerFrame]; 
  char *p_sFrame0 = new char [dst_pixPerFrame];
  char *p_sFrame1 = new char [dst_pixPerFrame];
  rgba_t *p_sFrameOut = new rgba_t [dst_pixPerFrame];

  std::ifstream ifs (inFnme. c_str (), std::ios::binary);
  if (!ifs) {
    std::cout << "Failed to open " << inFnme << std::endl;
    return 1;
  }
  std::ofstream ofs (outFnme. c_str (), std::ios::binary);
  if (!ofs) {
    std::cout << "Failed to create " << outFnme << std::endl;
    return 1;
  }

  int fStart = begFrame, fEnd=endFrame;
  int f = 0;
  bool hasUv = true;
  while (f < fEnd) {
    ifs. read (p_yBuf, in_yPerFrame);
    if (!ifs) {
      std::cout << "Failed to read y frame " << f << std::endl;
      break;
    }

    if (hasUv) {
      ifs. read (p_uvBuf, in_uvPerFrame);
      if (!ifs) {
        std::cout << "Failed to read uv frame " << f << std::endl;
        break;
      }
    }

    if (f<fStart) {++f; continue;};

    if (f==fStart) {
      getScaledFrame (p_yBuf, p_sFrame0, in_w, in_h, scale, padRows); 
      ++f;
      continue;
    } else if (f==fStart+1) {
      getScaledFrame (p_yBuf, p_sFrame1, in_w, in_h, scale, padRows); 
      ++f;
      continue;
    }

    char *p_tmp = p_sFrame0;
    p_sFrame0 = p_sFrame1;
    p_sFrame1 = p_tmp;
    getScaledFrame (p_yBuf, p_sFrame1, in_w, in_h, scale, padRows); 

    std::cout << "fpga_optflow Starting... Iter frame " << f << std::endl;
    // for now mywide_t and char are same
    fpga_optflow ( (mywide_t*)p_sFrame0, (mywide_t *)p_sFrame1, p_sFrameOut);
    std::cout << "fpga_optflow Done. Status " << std::endl;

    writeYuv444p (ofs, p_sFrameOut,  dst_w, dst_h);

    if (0) {
      std::stringstream ss;
      ss << "frame_" << f << ".ppm";
      savePpm (ss. str (), p_sFrameOut, dst_w, dst_h);

      ss. str ("");
      ss << "in0_" << f << ".ppm";
      savePpm (ss. str (), p_sFrame0, dst_w, dst_h);

      ss. str ("");
      ss << "in1_" << f << ".ppm";
      savePpm (ss. str (), p_sFrame1, dst_w, dst_h);

    }
    ++f;
  }

  delete [] p_yBuf;
  delete [] p_uvBuf;
  delete [] p_sFrame0;
  delete [] p_sFrame1;
  delete [] p_sFrameOut;

  ifs. close (); ofs. close ();
  std::cout << "test-cl: runClip Done." << std::endl;

  return EXIT_SUCCESS;
}
#endif

void unpackFrame (rgba2_t *p_pkd, rgba_t *p_frm, int width, int height, int wordsz)
{
  for (int r=0; r < height; ++r) {
    for (int c=0; c < width/wordsz; ++c) {
      p_frm->r = p_pkd->r0;
      p_frm->g = p_pkd->g0;
      p_frm->b = p_pkd->b0;
      p_frm->a = p_pkd->a0;
      ++p_frm;

      p_frm->r = p_pkd->r1;
      p_frm->g = p_pkd->g1;
      p_frm->b = p_pkd->b1;
      p_frm->a = p_pkd->a1;
      ++p_frm;

      ++p_pkd;
    }
  }

}

int runFramePair (int argc, char** argv)
{
  if (argc != 4) {
    std::cout << "Usage:\n\ttest-c.exe frame0.ppm frame1.ppm outframe.ppm\n" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "runFramePair started..." << std::endl;

  std::string f0Name (argv[1]), f1Name (argv[2]), f2Name (argv[3]);

  int height=MAX_HEIGHT, width=MAX_WIDTH;

  pix_t *p_f0 = readPpmY (f0Name, width, height);
  pix_t *p_f1 = readPpmY (f1Name, width, height);
  rgba_t *p_out = new rgba_t [width * height];

  //rgba2_t *p_pkdOut = new rgba2_t [width * height /WORD_SZ];
  rgba2_t *p_pkdOut = (rgba2_t *)sds_alloc_non_cacheable ( (width * height /WORD_SZ) * sizeof (rgba2_t));

  if (!p_f0 || !p_f1 || !p_out || !p_pkdOut) {
    std::cout << "Failed to alloc host memory" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "fpga_optflow Starting..." << std::endl;
  TIME_STAMP_INIT
  fpga_optflow ((mywide_t *)p_f0, (mywide_t *)p_f1, p_pkdOut);
  TIME_STAMP
  std::cout << "fpga_optflow Done." << std::endl;

  unpackFrame (p_pkdOut, p_out, width, height, WORD_SZ);
  if (! savePpm (f2Name, p_out, width, height)) {
    return EXIT_FAILURE;
  }
 
  //delete [] p_f0; 
  //delete [] p_f1; 
  delete [] p_out;
  //delete [] p_pkdOut;

  sds_free (p_f0);
  sds_free (p_f1);
  sds_free (p_pkdOut);

  std::cout << "runFramePair Done." << std::endl;
  return EXIT_SUCCESS;
}

#if 0
int runOpenCvClip (int argc, char **argv)
{
  std::string clip = argv [1];
  cv::VideoCapture cam (clip);
  if (! cam. isOpened ()) {
    std::cout << "Failed to open clip " << clip << std::endl;
    return 1;
  }
  cv::Mat gray0;
  bool capStatus=false;
  int fcnt=0;
  int maxFrames = 1000;

  int height = MAX_HEIGHT, width = MAX_WIDTH;
  int nPixels = width * height;
  pix_t *p_f0 = new pix_t [nPixels];
  pix_t *p_f1 = new pix_t [nPixels];
  rgba_t *p_out = new rgba_t [nPixels];
  rgb_t  *p_outRgb  = new rgb_t [nPixels];

  while (fcnt < maxFrames) {
    cv::Mat frame, gray;
    capStatus = cam. read (frame);
    if (!capStatus) { 
      std::cout << "Failed to grab frame " << fcnt << std::endl;
      return 1;
    }
    ++fcnt;
    
    cv::cvtColor (frame, gray, CV_BGR2GRAY);
    if (fcnt == 1) {
      if (gray. rows != MAX_HEIGHT || gray. cols != MAX_WIDTH) {
        std::cout << "Dim mismatch: Got (" << gray. rows << "," << gray. cols << ")" << std::endl;
        return 1;
      }

      for (int row=0; row < height; ++row) {
        const unsigned char *p_img = gray. ptr <unsigned char>(row);
        for (int col=0; col < width; ++col) {
          unsigned char pix = *p_img; ++p_img;
          p_f0 [row * width + col] = pix;
        }
      }

      continue;
    }

    // convert gray0 and gray to raw arrays and call fpga_optflow
    for (int row=0; row < height; ++row) {
      const unsigned char *p_img = gray. ptr <unsigned char>(row);
      for (int col=0; col < width; ++col) {
        unsigned char pix = *p_img; ++p_img;
        p_f1 [row * width + col] = pix;
      }
    }

    std::cout << "fpga_optflow Starting..." << std::endl;
    fpga_optflow ((mywide_t *)p_f0, (mywide_t *)p_f1, p_out);
    std::cout << "fpga_optflow Done." << std::endl;

    {
      for (int i=0; i<nPixels; ++i) {
        rgb_t rgb;
        rgb.r = p_out [i].r; rgb.g = p_out [i].g; rgb.b = p_out [i].b;
        p_outRgb [i] = rgb;
      }
    }

    // convert p_out to Mat and imshow
    cv::Mat flowViz = cv::Mat (height, width, CV_8UC3, (unsigned char*)p_outRgb, cv::Mat::AUTO_STEP);

    cv::imshow ("Flow Viz", flowViz);
    cv::waitKey (10);

    pix_t *p_tmp = p_f0;
    p_f0 = p_f1;
    p_f1 = p_tmp;
  }

  delete [] p_f0;
  delete [] p_f1;
  delete [] p_out;
  delete [] p_outRgb;

  std::cout << "runOpenCvClip Done." << std::endl;
  return 0;
}

int runOpenCvClip2Clip (int argc, char **argv)
{
  std::string clip = argv [1];
  std::string outClip = argv [2];
  int maxFrames = std::atoi (argv [3]);
  int dummyArg = std::atoi (argv [4]);

  std::cout << "Processing in-clip " << clip << " to out-clip " << outClip << " maxFrames " << maxFrames << std::endl;
  cv::VideoCapture cam (clip);
  if (! cam. isOpened ()) {
    std::cout << "Failed to open clip " << clip << std::endl;
    return 1;
  }

  // get format
  int ex = static_cast<int>(cam.get(CV_CAP_PROP_FOURCC));
  char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
  cv::Size S = cv::Size((int) cam.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                        (int) cam.get(CV_CAP_PROP_FRAME_HEIGHT));
  std::cout << "Clip " << clip << " dims " << S << " Code type " << EXT << " ex " << ex << std::endl;
  cv::VideoWriter writer;

  //writer.open (outClip, ex, cam.get(CV_CAP_PROP_FPS), S, true);
  //writer.open (outClip, CV_FOURCC('U', '2', '6', '3'), cam.get(CV_CAP_PROP_FPS), S, true);
  writer.open (outClip, CV_FOURCC('P','I','M','1'), cam.get(CV_CAP_PROP_FPS), S, true);
  if (!writer. isOpened()) {
    std::cout << "Failed to open output clip" << std::endl;
    return 1;
  }

  cv::Mat gray0;
  bool capStatus=false;
  int fcnt=0;

  int height = MAX_HEIGHT, width = MAX_WIDTH;
  int nPixels = width * height;
  pix_t *p_f0 = new pix_t [nPixels];
  pix_t *p_f1 = new pix_t [nPixels];
  rgba_t *p_out = new rgba_t [nPixels];
  rgb_t  *p_outRgb = new rgb_t [nPixels];

  while (fcnt < maxFrames) {
    cv::Mat frame, gray;
    capStatus = cam. read (frame);
    if (!capStatus) { 
      std::cout << "Failed to grab frame " << fcnt << std::endl;
      return 1;
    }
    ++fcnt;
    
    cv::cvtColor (frame, gray, CV_BGR2GRAY);
    if (fcnt == 1) {
      if (gray. rows != MAX_HEIGHT || gray. cols != MAX_WIDTH) {
        std::cout << "Dim mismatch: Got (" << gray. rows << "," << gray. cols << ")" << std::endl;
        return 1;
      }

      for (int row=0; row < height; ++row) {
        const unsigned char *p_img = gray. ptr <unsigned char>(row);
        for (int col=0; col < width; ++col) {
          unsigned char pix = *p_img; ++p_img;
          p_f0 [row * width + col] = pix;
        }
      }

      continue;
    }

    // convert gray0 and gray to raw arrays and call fpga_optflow
    for (int row=0; row < height; ++row) {
      const unsigned char *p_img = gray. ptr <unsigned char>(row);
      for (int col=0; col < width; ++col) {
        unsigned char pix = *p_img; ++p_img;
        p_f1 [row * width + col] = pix;
      }
    }

    std::cout << "fpga_optflow Starting... frame " << fcnt << std::endl;
    fpga_optflow ((mywide_t *)p_f0, (mywide_t *)p_f1, p_out);
    std::cout << "fpga_optflow Done." << std::endl;

    {
      for (int i=0; i<nPixels; ++i) {
        rgb_t rgb;
        rgb.r = p_out [i].r; rgb.g = p_out [i].g; rgb.b = p_out [i].b;
        p_outRgb [i] = rgb;
      }
    }

    // convert p_out to Mat and imshow
    cv::Mat flowViz = cv::Mat (height, width, CV_8UC3, (unsigned char*)p_outRgb, cv::Mat::AUTO_STEP);

    //cv::imshow ("Flow Viz", flowViz);
    //cv::waitKey (10);
    writer << flowViz;

    pix_t *p_tmp = p_f0;
    p_f0 = p_f1;
    p_f1 = p_tmp;
  }

  delete [] p_f0;
  delete [] p_f1;
  delete [] p_out;
  delete [] p_outRgb;

  std::cout << "runOpenCvClip2Clip Done. Output in " << outClip << std::endl;
  return 0;
}
#endif

int main (int argc, char** argv)
{
    if (argc == 4) {
      return runFramePair (argc, argv);
    } else {
      std::cout << "\ttest-c.exe frame0.ppm frame1.ppm outframe.ppm\n" << std::endl;
      return 1;
    }
#if 0
  if (argc == 7) {
    return runClip (argc, argv);
  } else if (argc == 4) {
    return runFramePair (argc, argv);
  } else if (argc == 2) {
    return runOpenCvClip (argc, argv);
  } else if (argc == 5) {
    return runOpenCvClip2Clip (argc, argv);
  } else {
    std::cout << "Usage:\n\ttest-cl.exe xclbin inclip.yuv outclip.yuv width height begFrame endFrame\n\tOut clip dims: " 
              << MAX_WIDTH << "x" << MAX_HEIGHT << "\n" << std::endl;
    std::cout << "\ttest-cl.exe xclbin frame0.ppm frame1.ppm outframe.ppm\n" << std::endl;
    return 1;
  }
#endif
}
