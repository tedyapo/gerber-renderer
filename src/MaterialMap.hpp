#ifndef MATERIALMAP_HPP_INCLUDED_
#define MATERIALMAP_HPP_INCLUDED_

#include "Material.hpp"
#include <gerbv.h>
#include "Image.h"

// a multi-plane texture map for per-pixel materials
// ( ambient, diffuse, specular, emission, transparency and shininess )
//
// !!! todo: break with fixed pipeline conventions:
//   make amb, diff, spec, emiss all 3-vectors; have 1d textures for 
//     transparency and shininess.  if these all fit into texture units
//
// todo: move these calculations to the GPU
//
class MaterialMap
{
public:
  MaterialMap(int rows, int cols, bool clear_flag = true);
  void Clear();

  // initialize to uniform material
  void Initialize(Material const& material);

  //
  // add a material modulated by a cairo surface
  // note: red channel is used as alpha to mix with with existing material
  //       transparency is not mixed here (how should it be handled?)
  void AddCairoMaterial(cairo_surface_t *cSurface, Material const& material,
                        bool invert);

  //
  // forcibly set transparency, overwriting existing values
  //
  void setCairoTransparency(cairo_surface_t *cSurface);

  //
  // "add" to existing transparency, keeping most transparent value
  //
  void addCairoTransparency(cairo_surface_t *cSurface);

  //
  // get images for various component textures
  //
  Image<RGB> const& getAmbientTexture();
  Image<RGBA> const& getDiffuseTexture();
  Image<RGBA> const& getSpecularTexture();
  Image<RGB> const& getEmissionTexture();

private:
  Image<v3d> ambient;
  Image<v3d> diffuse;
  Image<v3d> specular;
  Image<v3d> emission;
  Image<double> transparency;
  Image<double> shininess;
  Image<RGB> texture3;
  Image<RGBA> texture4;

  template <typename Pixel_t>
  void AddCairoMaterialComponent(Image<Pixel_t> &component,
                                 cairo_surface_t *cSurface,
                                 Pixel_t value, double transparency,
                                 bool invert);
};



#endif // #ifndef MATERIALMAP_HPP_INCLUDED_
