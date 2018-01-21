#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

/*
todo:

[ ] clean io to use C++ iostream
[ ] remove deprecated functions
[ ] move gamma functions etc to ImageUtils.hpp, also typedefs for pixel_t's
[ ] rename image.h, imageops.h to hpp
[ ] remove old functor interface
[ ] add stride to allow sub-images
[ ] create imageview/image or similar pair
[ ] allow imageview creation from existing image for sub-images
[ ] track sub-images and notify on delete
*/

//
// Image.h - simple template image class
// first official version 7/4/08/tcy

//
// #define DEBUG before including this file to turn on (slow) bounds checking
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <cctype>
#include <cassert>
#include <limits>
#include <math.h>
#include "util.h"
#include "VectorN.hpp"

typedef uint8_t byte;
typedef VectorN<byte, 3> RGB;
typedef VectorN<byte, 3> sRGB;
typedef VectorN<byte, 3> RGB24;
typedef VectorN<byte, 4> RGBA;
typedef VectorN<byte, 4> RGBA32;

//
// simple function to clamp values into (0,1)
//
inline double sRGBclamp(double c){
  if (c < 0.0) return 0.0;
  if (c > 1.0) return 1.0;
  return c;
}

//
// sRGB gamma function
// data and algorithm from:
// http://en.wikipedia.org/wiki/SRGB_color_space
//
inline double sRGBgamma(double c){
  if (c < 0.0031308){
    return 12.92 * c;
  } else {
    return 1.055 * pow(c, 1/2.4) - 0.055;
  }
}

//
// BT REC709 gamma function
//
inline double REC709gamma(double c){
  if (c <= 0.018){
    return 4.5 * c;
  } else {
    return (1.099 * pow(c, 0.45)) - 0.099;
  }
}

//
// BT REC709 inverse gamma function
//
inline double REC709degamma(double c){
  if (c <= 0.081){
    return c / 4.5;
  } else {
    return pow((c+0.099)/1.099, 1./0.45);
  }
}

//
// scale 0-1 rgb data to 0-255 with proper rounding
//
inline double sRGB255(double c){
  c = floor(255. * c + 0.5);
  if (c > 255.) return 255.;
  return c;
}

//
// sRGB de-gamma function
// data and algorithm from:
// http://en.wikipedia.org/wiki/SRGB_color_space
//
inline double sRGBdegamma(double c){
  if (c < 0.04045){
    return c / 12.92;
  } else {
    return pow((c + 0.055)/1.055, 2.4);
  }
}

//
// virtual base class for point-operator functors
//
// deprecated 6/3/0/08: template version is faster
//
template <typename Pixel>
class ImagePointFunctor
{
 public:
  virtual void operator()(Pixel& pixel, int row, int col) = 0;
  virtual ~ImagePointFunctor(){};
};

// get a number from a PNM file header
static int get_pnm_number(FILE *fp){
  enum {RESET, NUMBER, COMMENT} state;
  state = RESET;
  int value = 0;
  bool done = false;
  while (!done){
    int c = getc(fp);
    if (isspace(c)){
      switch(state){
      case RESET:
        break;
      case NUMBER:
        done = true;
        break;
      case COMMENT:
        if ('\n' == c ||
            '\r' == c)
          state = RESET;
        break;
      }
    } else if (isdigit(c)){
      switch (state){
      case RESET:
        state = NUMBER;
        // fall through
      case NUMBER:
        value = 10 * value + (c - '0');      
        break;
      case COMMENT:
        break;
      }
    } else if ('#' == c){
      state = COMMENT;
    }
  }
  return value;
}


// traits for PNM images
template <typename Pixel>
struct PixelTraits { 
  typedef Pixel Pixel_t;
  typedef Pixel base_Pixel_t;
  typedef Pixel accum_Pixel_t;
  typedef Pixel base_accum_Pixel_t;

  // "P0" magic is used for generic non-standard types
  const char *magic(){ return "P0"; }
  int pnm_maxval(){ return 0; }
  static Pixel maximum_value(){
    return std::numeric_limits<Pixel>::max(); 
  };
  static Pixel minimum_value(){
    return std::numeric_limits<Pixel>::min();
  };
};

template <>
struct PixelTraits<byte> {
  typedef byte Pixel_t;
  typedef byte base_Pixel_t;
  typedef double accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic(){ return "P5"; }
  int pnm_maxval(){ return 255; }
  static byte maximum_value(){
    return 255;
  }
  static byte minimum_value(){
    return 0;
  }
};

template <>
struct PixelTraits<sRGB> {
  typedef VectorN<byte, 3> Pixel_t;
  typedef byte base_Pixel_t;
  typedef v3d accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic(){ return "P6"; }
  int pnm_maxval(){ return 255; }
  static sRGB maximum_value(){
    return sRGB(255, 255, 255);
  }
  static sRGB minimum_value(){
    return sRGB(0, 0, 0);
  }
};

template <>
struct PixelTraits<v3d> {
  typedef v3d Pixel_t;
  typedef double base_Pixel_t;
  typedef v3d accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic(){ return "P0"; }
  int pnm_maxval(){ return 0; }
  static v3d maximum_value(){
    return v3d(std::numeric_limits<double>::max());
  }
  static v3d minimum_value(){
    return v3d(-std::numeric_limits<double>::max());
  }
};

template <>
struct PixelTraits<unsigned short> {
  typedef unsigned short Pixel_t;
  typedef unsigned short base_Pixel_t;
  typedef double accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic(){ return "P5"; }
  int pnm_maxval(){ return 65535; }
  static unsigned short maximum_value(){
    return 65535U;
  }
  static unsigned short minimum_value(){
    return 0;
  }
};

// !!! this is temporary: need a better way to handle these images
template <>
struct PixelTraits<VectorN<unsigned short, 3> > {
  typedef VectorN<unsigned short, 3> Pixel_t;
  typedef unsigned short base_Pixel_t;
  typedef VectorN<unsigned short, 3> accum_Pixel_t;
  typedef unsigned short base_accum_Pixel_t;
  const char *magic(){ return "P6"; }
  int pnm_maxval(){ return 65535; }
};

template <>
struct PixelTraits<int> {
  typedef int Pixel_t;
  typedef int base_Pixel_t;
  typedef int accum_Pixel_t;
  typedef int base_accum_Pixel_t;
  const char *magic(){ return "P0"; }
  int pnm_maxval(){ return 65535; }
  static int maximum_value(){
    return std::numeric_limits<int>::max();
  }
  static int minimum_value(){
    return std::numeric_limits<int>::min();
  }
};

//
// generic image class
//
template <typename Pixel>
class Image
{
 public:
  typedef Pixel Pixel_t;
  typedef typename PixelTraits<Pixel_t>::base_Pixel_t base_Pixel_t;
  typedef typename PixelTraits<Pixel_t>::accum_Pixel_t accum_Pixel_t;
  typedef typename PixelTraits<Pixel_t>::base_accum_Pixel_t base_accum_Pixel_t;

  Image(){
    rows = 0;
    cols = 0;
    min_x = max_x = min_y = max_y = 0.0;
    data = NULL;
  }

  Image(int rows, int cols){
    this->rows = rows;
    this->cols = cols;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    data = new Pixel[rows*cols];
  }

  Image(int rows, int cols, Pixel* toCopy){
    this->rows = rows;
    this->cols = cols;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    data = toCopy;
  }

  Image(int rows, int cols, double min_x, double min_y,
        double max_x, double max_y){
    this->rows = rows;
    this->cols = cols;
    this->min_x = min_x;
    this->max_x = max_x;
    this->min_y = min_y;
    this->max_y = max_y;
    data = new Pixel[rows*cols];
  }

  //!!! TODO merge this with read() below
  Image(const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp){
      FATAL_ERROR("unable to open file %s", filename);
    }
    load_from_fp(fp);
    fclose(fp);
  }

  Image(FILE *fp){
    load_from_fp(fp);
  }
  
  Image(const Image &i){
    data = NULL;
    operator=(i);
  }

  Image& operator= (const Image &i){
    if (this == &i){
      return *this;
    }
    if (rows != i.rows || cols != i.cols){
      delete [] data;
      data = NULL;
    }
    if (NULL == data){
      data = new Pixel[i.rows * i.cols];
    }
    rows = i.rows;
    cols = i.cols;
    min_x = i.min_x;
    min_y = i.min_y;
    max_x = i.max_x;
    max_y = i.max_y;
    for (int j=0; j<rows*cols; j++){
      data[j] = i.data[j];
    }
    return *this;
  }

  ~Image(){
    if (NULL != data){
      delete[] data;
    }
  }

  void read(const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp){
      FATAL_ERROR("unable to open file %s", filename);
    }

    char magic[2];
    fread(magic, 2, 1, fp);
    if (strncmp(magic, traits.magic(), 2)){
      FATAL_ERROR("Incorrect file type '%c%c'", magic[0], magic[1]);
    }
    int new_cols = get_pnm_number(fp);
    int new_rows = get_pnm_number(fp);
    int maxval = get_pnm_number(fp);
    if (maxval != traits.pnm_maxval()){
      FATAL_ERROR("Incorrect maxval %d", maxval);
    }
    if (cols != new_cols || rows != new_rows){
      delete[] data;
      data = new Pixel[new_rows*new_cols];
    }
    rows = new_rows;
    cols = new_cols;
    fread(data, rows, cols*sizeof(Pixel), fp);
    fclose(fp);
  }

  void write(const char *filename, const char *comment = NULL){
    FILE *fp;
    if (NULL == filename){
      fp = stdout;
    } else {
      fp = fopen(filename, "wb");
    }
    if (NULL == fp){
      FATAL_ERROR("Unable to open %s for writing", filename);
    }
    fprintf(fp, "%s\n", traits.magic());
    if (NULL != comment){
      fprintf(fp, "#%s\n", comment);
    }
    fprintf(fp,"%d %d\n%d\n",
            cols, rows, traits.pnm_maxval());
    fwrite(data, rows, cols*sizeof(Pixel), fp);
    fclose(fp);
  }

  // deprecated 6/30/08 - use write() above
  void writePPM(char *filename) __attribute__ ((deprecated)) {
    FILE *fp = fopen(filename, "wb");
    if (NULL == fp){
      FATAL_ERROR("Unable to open %s for writing\n", filename);
    }
    fprintf(fp,"%s\n%d %d\n%d\n",
            traits.magic(), cols, rows, traits.pnm_maxval());
    fwrite(data, rows, cols*sizeof(Pixel), fp);
    fclose(fp);
  }

  int getRows() const {return rows;}
  int getCols() const {return cols;}

  int linear_index(int row, int col){
    return row*cols+col;
  }

  Pixel& operator()(int row, int col){
#ifdef DEBUG
    if (row>=0 && row<rows && col>=0 && col<cols){
#endif
      return data[row*cols+col];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d, %d) addressed, accessing fake pixel",
              row, col);
      return fake_pixel;
    }
#endif
  }

  Pixel operator()(int row, int col) const {
#ifdef DEBUG
    if (row>=0 && row<rows && col>=0 && col<cols){
#endif
      return data[row*cols+col];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d, %d) addressed, accessing fake pixel",
              row, col);
      return fake_pixel;
    }
#endif
  }

  Pixel operator()(int linear_index) const {
#ifdef DEBUG
    if (linear_index >=0 && linear_index < rows*cols){
#endif
      return data[linear_index];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d) addressed, accessing fake pixel",
              linear_index);
      return fake_pixel;
    }
#endif
  }

  Pixel &operator()(int linear_index) {
#ifdef DEBUG
    if (linear_index >=0 && linear_index < rows*cols){
#endif
      return data[linear_index];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d) addressed, accessing fake pixel",
              linear_index);
      return fake_pixel;
    }
#endif
  }


  // use bilinear interpolation to get valeus at non-integer coords
  // !!!note: this only works for scalar images now; modify for use on vectors
  Pixel bilinear(double row, double col){
    int rmin = int(floor(row));
    if (rmin < 0) rmin = 0;
    if (rmin > rows - 1){
      rmin = rows - 1;
    }
    int rmax = rmin + 1;
    if (rmax < 0) rmax = 0;
    if (rmax > rows - 1){
      rmax = rows - 1;
    }
    row -= rmin;
    int cmin = int(floor(col));
    if (cmin < 0) cmin = 0;
    if (cmin > cols - 1){
      cmin = cols - 1;
    }
    int cmax = cmin + 1;
    col -= cmin;
    if (cmax < 0) cmax = 0;
    if (cmax > cols - 1){
      cmax = cols - 1;
    }

    accum_Pixel_t i1 = (*this)(rmin, cmin);
    accum_Pixel_t i2 = (*this)(rmin, cmax);
    accum_Pixel_t i12 = (1. - col) * i1 + col * i2;
    accum_Pixel_t i3 = (*this)(rmax, cmin);
    accum_Pixel_t i4 = (*this)(rmax, cmax);
    accum_Pixel_t i34 = (1. - col) * i3 + col * i4;
    accum_Pixel_t i = (1. - row) * i12 + row * i34;
    return Pixel_t(i);
  }

#if 0
  //!!! should these be reversed like this?
  Pixel& operator()(double x, double y)
    {
      int col = int(cols*(x-min_x)/(max_x-min_x));
      int row = int(rows*(max_y - y)/(max_y-min_y));
#ifdef DEBUG
      if (row >= 0 && row < rows && col >= 0 && col < cols){
#endif
        return data[row*cols+col];
#ifdef DEBUG
      } else {
        WARNING("Invalid pixel (%d, %d) addressed, accessing fake pixel",
                row, col);
        return fake_pixel;
      }
#endif
    }
#endif

  int row(double y){
    return int(rows*(max_y-y)/(max_y-min_y));    
  }

  int col(double x){
    return int(cols*(x-min_x)/(max_x-min_x));
  }

  Pixel* getData(){return data;};
  const Pixel* getData() const {return data;};


  // apply a supplied point-operator functor to each pixel
  // deprecated due to faster template version below 6/30/08
  void applyPointFunctor(ImagePointFunctor<Pixel>& functor) __attribute__ ((deprecated))
  {
    for (int row=0; row<rows; row++){
      for (int col=0; col<cols; col++){
        functor((*this)(row,col), row, col);
      }
    }
  }


  // use Bresenham's algorithm to apply a point functor to
  //  all pixels along a line in the image
  void applyFunctorOnLine(ImagePointFunctor<Pixel>& functor, 
                          int r2, int c2, int r1, int c1){
    if (r1 == r2 && c1 == c2){
      functor((*this)(r1,c1), r1, c1);
      return;
    }
    int row, col, err;
    int dr = r1 - r2;
    int dc = c1 - c2;
    int rstep = SGN(dr);
    int cstep = SGN(dc);
    dr = abs(dr);
    dc = abs(dc);
    if (dr > dc){
      col = c2;
      err = - dr>>1;
      for (row=r2; row!=r1; row+=rstep){
        functor((*this)(row,col), row, col);
        err += dc;
        if (err > 0){
          col += cstep;
          err -= dr;
        }
      }
    } else {
      row = r2;
      err = - dc>>1;
      for (col=c2; col!=c1; col+=cstep){
        functor((*this)(row,col), row, col);
        err += dr;
        if (err > 0){
          row += rstep;
          err -= dc;
        }
      }
    }
  }
  
 private:
  int rows;
  int cols;
  double min_x, min_y;
  double max_x, max_y;
  Pixel *data;
  Pixel fake_pixel;
  PixelTraits<Pixel> traits;
  int SGN(int x) {return (x)<0?-1:(x)>0?1:0; }
  size_t size;
  
  void load_from_fp(FILE *fp){
    char magic[2];
    size = fread(magic, 2, 1, fp);
    if (size != 2 || strncmp(magic, traits.magic(), 2)){
      FATAL_ERROR("Incorrect file type '%c%c'", magic[0], magic[1]);
    }
    cols = get_pnm_number(fp);
    rows = get_pnm_number(fp);
    int maxval = get_pnm_number(fp);
    if (maxval != traits.pnm_maxval()){
      FATAL_ERROR("Incorrect maxval %d\n", maxval);
    }
    data = new Pixel[rows*cols];
    size = fread(data, rows, cols*sizeof(Pixel), fp);


    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
  }
};

//
// convert one image type to another via pixel-wise casts
//
template <class T1, class T2>
  void convertImage(Image<T1> &src, Image<T2> &dst){
  assert(src.getRows() == dst.getRows());
  assert(src.getCols() == dst.getCols());
  for (int row=0; row<src.getRows(); row++){
    for (int col=0; col<src.getCols(); col++){
      dst(row,col) = T2(src(row,col));
    }
  }
}

// apply a supplied point-operator functor to each pixel
//template <typename Pixel, template <typename> class Operator >
//void ImagePointOperation(Image<Pixel> &image, Operator<Pixel> &operation){

template <typename Pixel, typename Operator >
  void ImagePointOperation(Image<Pixel> &image, Operator &operation){
  Pixel *pixel = image.getData();
  for (int row=0; row<image.getRows(); row++){
    for (int col=0; col<image.getCols(); col++){
      operation(*pixel++, row, col);
    }
  }
}

#endif // #ifndef IMAGE_H_INCLUDED

