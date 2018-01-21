#include "BumpMap.hpp"
#include "ImageOps.h"
#include <omp.h>

BumpMap::
BumpMap(int rows, int cols, double xscale, double yscale, bool clear_flag)
  : height_map(rows, cols),
    normal_map(rows, cols),
    xscale(xscale),
    yscale(yscale),
    normal_calculated(false)
{
  // !!! assert that rows >= 3 && cols >= 3
  if (clear_flag)
  {
    this->Clear();
  }
}

void
BumpMap::
Clear(double value)
{
  ImageClearer<double> clearer(value);
  ImagePointOperation(height_map, clearer);

  normal_calculated = false;
}

//
// add a height field directly from a cairo surface
// note: alpha channel is used to modulate height for antialiasing
//       assumes height is in red channel
//       thickness in same units as this->scale (e.g. inches)
//
void
BumpMap::
AddCairoHeightField(cairo_surface_t *cSurface, double thickness)
{
  int c_rows = cairo_image_surface_get_height(cSurface);
  int c_cols = cairo_image_surface_get_width(cSurface);
  if (c_rows != height_map.getRows() ||
      c_cols != height_map.getCols())
  {
    //!! assert here
  }
  
  int stride = cairo_image_surface_get_stride(cSurface);
  byte *argb = cairo_image_surface_get_data(cSurface);
  
  double factor = thickness / (255. * 255.);

  for (int row=0; row<height_map.getRows(); ++row)
  {
    byte *p_alpha  = argb + row*stride + 0; // alpha channel
    byte *p_height = argb + row*stride + 1; // red channel
    for (int col=0; col<height_map.getCols(); ++col)
    {
      height_map(row, col) += (factor * *p_height) * *p_alpha;
      p_alpha += 4;
      p_height += 4;
    }
  }
  
  normal_calculated = false;
}


Image<sRGB> const&
BumpMap::
getNormalMap()
{
  if (!normal_calculated)
  {
    CalculateNormal();
  }
  return normal_map;
}

void 
BumpMap::
CalculateNormal()
{
  // calculate normals for interior of map
  #pragma omp for
  for (int row=1; row<height_map.getRows()-1; ++row){
    for (int col=1; col<height_map.getCols()-1; ++col){
      
      //    h1
      // h2    h3
      //    h4
      double h1 = height_map(row - 1, col + 0);
      double h2 = height_map(row + 0, col - 1);
      double h3 = height_map(row + 0, col + 1);
      double h4 = height_map(row + 1, col + 0);
      
      v3d v1(-2. * xscale, 0., h3-h2);
      v3d v2(0., -2. * yscale, h1-h4);
      v3d n = cross(v1, v2);
      n.normalize();
      normal_map(row, col).r() = clamp(127.5 + 127.5 * n.x(), 0., 255.);
      normal_map(row, col).g() = clamp(127.5 + 127.5 * n.y(), 0., 255.);
      normal_map(row, col).b() = clamp(127.5 + 127.5 * n.z(), 0., 255.);
    }
  }
  
  // fix edge pixels by copying nearby calculated normal
  #pragma omp for
  for (int row=0; row<normal_map.getRows(); ++row)
  {
    normal_map(row, 0) = normal_map(row, 1);
    normal_map(row, normal_map.getCols() - 1) = 
      normal_map(row, normal_map.getCols() - 2);
  }
  #pragma omp for
  for (int col=0; col<normal_map.getCols(); ++col)
  {
    normal_map(0, col) = normal_map(1, col);
    normal_map(normal_map.getRows() - 1, col) =
      normal_map(normal_map.getRows() - 2, col);
  }
  
  normal_calculated = true;
}
