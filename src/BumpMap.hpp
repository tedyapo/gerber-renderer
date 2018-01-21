#ifndef BUMPMAP_HPP_INCLUDED_
#define BUMPMAP_HPP_INCLUDED_

#include <gerbv.h>
#include "Image.h"

// class for creating a height map and transforming into a normal map
// for bump-map texturing
//
// !!! add x-, y-scale values for non-square boards with POT textures
//
class BumpMap
{
public:
  BumpMap(int rows, int cols, double xscale, double yscale,
          bool clear_flag = true);
  void Clear(double value = 0.);

  //
  // add a height field directly from a cairo surface
  // note: alpha channel is used to modulate height for antialiasing
  //       assumes height is in red channel
  //       thickness in same units as this->scale (e.g. inches)
  //
  void AddCairoHeightField(cairo_surface_t *cSurface, double thickness);

  Image<sRGB> const& getNormalMap();
  void CalculateNormal();

private:
  Image<double> height_map;
  Image<RGB> normal_map;
  // units (e.g. inches) per pixel
  double xscale, yscale; 
  bool normal_calculated;
};

#endif // #ifndef BUMPMAP_HPP_INCLUDED_
