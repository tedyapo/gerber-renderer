#include "MaterialMap.hpp"
#include "ImageOps.h"
#include <omp.h>

MaterialMap::
MaterialMap(int rows, int cols, bool clear_flag)
  : ambient(rows, cols),
    diffuse(rows, cols),
    specular(rows, cols),
    emission(rows, cols),
    transparency(rows, cols),
    shininess(rows, cols),
    texture3(rows, cols),
    texture4(rows, cols)
{
  if (clear_flag)
  {
    this->Clear();
  }
};

void
MaterialMap::
Clear()
{
  ImageClearer<v3d> v3d_clearer(v3d(0., 0., 0.));
  ImagePointOperation(ambient, v3d_clearer);
  ImagePointOperation(diffuse, v3d_clearer);
  ImagePointOperation(specular, v3d_clearer);
  ImagePointOperation(emission, v3d_clearer);
  
  ImageClearer<double> d_clearer(0.);
  ImagePointOperation(transparency, d_clearer);
  ImagePointOperation(shininess, d_clearer);
}

// initialize to uniform material
void
MaterialMap::
Initialize(Material const& material)
{
  ImageClearer<v3d> ambient_clearer(material.getAmbient());
  ImagePointOperation(ambient, ambient_clearer);
  ImageClearer<v3d> diffuse_clearer(material.getDiffuse());
  ImagePointOperation(diffuse, diffuse_clearer);
  ImageClearer<v3d> specular_clearer(material.getSpecular());
  ImagePointOperation(specular, specular_clearer);
  ImageClearer<double> transparency_clearer(material.getTransparency());
  ImagePointOperation(transparency, transparency_clearer);
  ImageClearer<double> shininess_clearer(material.getShininess());
  ImagePointOperation(shininess, shininess_clearer);
}

//
// add a material modulated by a cairo surface
// note: red channel is used as alpha to mix with with existing material
//       transparency is not mixed here (how should it be handled?)
void
MaterialMap::
AddCairoMaterial(cairo_surface_t *cSurface, Material const& material,
                 bool invert)
{
  int c_rows = cairo_image_surface_get_height(cSurface);
  int c_cols = cairo_image_surface_get_width(cSurface);
  if (c_rows != ambient.getRows() ||
      c_cols != ambient.getCols())
  {
    //!!! assert here
  }
  
  AddCairoMaterialComponent(ambient, cSurface,
                            material.getAmbient(),
                            material.getTransparency(),
                            invert);
  AddCairoMaterialComponent(diffuse, cSurface,
                            material.getDiffuse(), 
                            material.getTransparency(),
                            invert);
  AddCairoMaterialComponent(specular, cSurface,
                            material.getSpecular(),
                            material.getTransparency(),
                            invert);
  AddCairoMaterialComponent(emission, cSurface,
                            material.getEmission(),
                            material.getTransparency(),
                            invert);
  AddCairoMaterialComponent(shininess, cSurface,
                            material.getShininess(),
                            material.getTransparency(),
                            invert);
}

//
// forcibly set transparency, overwriting existing values
//
void
MaterialMap::
setCairoTransparency(cairo_surface_t *cSurface)
{
  int c_rows = cairo_image_surface_get_height(cSurface);
  int c_cols = cairo_image_surface_get_width(cSurface);
  if (c_rows != transparency.getRows() ||
      c_cols != transparency.getCols())
  {
    //!!! assert here
  }
  
  int stride = cairo_image_surface_get_stride(cSurface);
  byte *argb = cairo_image_surface_get_data(cSurface);
  
  double inv255 = 1. / 255.;
  #pragma omp for
  for (int row=0; row<transparency.getRows(); ++row)
  {
    byte *pargb = argb + row*stride + 1; // point to red channel
    for (int col=0; col<transparency.getCols(); ++col)
    {
      transparency(row, col) = inv255 * (255 - *pargb);
      pargb += 4;
    }
  }
}

//
// "add" to transparency, keeping most transparent value
//
void
MaterialMap::
addCairoTransparency(cairo_surface_t *cSurface)
{
  int c_rows = cairo_image_surface_get_height(cSurface);
  int c_cols = cairo_image_surface_get_width(cSurface);
  if (c_rows != transparency.getRows() ||
      c_cols != transparency.getCols())
  {
    //!!! assert here
  }
  
  int stride = cairo_image_surface_get_stride(cSurface);
  byte *argb = cairo_image_surface_get_data(cSurface);
  
  double inv255 = 1. / 255.;
  #pragma omp for
  for (int row=0; row<transparency.getRows(); ++row)
  {
    byte *pargb = argb + row*stride + 1; // point to red channel
    for (int col=0; col<transparency.getCols(); ++col)
    {
      transparency(row, col) = std::min(transparency(row, col),
                                        inv255 * (255 - *pargb));
      pargb += 4;
    }
  }
}

Image<RGB> const& 
MaterialMap::
getAmbientTexture()
{
  #pragma omp for
  for (int row=0; row<texture3.getRows(); ++row)
  {
    for (int col=0; col<texture3.getCols(); ++col)
    {
      texture3(row, col).r() = clamp(255. * ambient(row, col).r(), 0., 255.);
      texture3(row, col).g() = clamp(255. * ambient(row, col).g(), 0., 255.);
      texture3(row, col).b() = clamp(255. * ambient(row, col).b(), 0., 255.);
    }
  }
  return texture3;
}

Image<RGBA> const& 
MaterialMap::
getDiffuseTexture()
{
  #pragma omp for
  for (int row=0; row<texture4.getRows(); ++row)
  {
    for (int col=0; col<texture4.getCols(); ++col)
    {
      texture4(row, col).r() = clamp(255. * diffuse(row, col).r(), 0., 255.);
      texture4(row, col).g() = clamp(255. * diffuse(row, col).g(), 0., 255.);
      texture4(row, col).b() = clamp(255. * diffuse(row, col).b(), 0., 255.);
      texture4(row, col).a() = clamp(255. * transparency(row, col),
                                   0., 255.);
    }
  }
  return texture4;
}

Image<RGBA> const& 
MaterialMap::
getSpecularTexture()
{
  #pragma omp for
  for (int row=0; row<texture4.getRows(); ++row)
  {
    for (int col=0; col<texture4.getCols(); ++col)
    {
      texture4(row, col).r() = clamp(255. * specular(row, col).r(), 0., 255.);
      texture4(row, col).g() = clamp(255. * specular(row, col).g(), 0., 255.);
      texture4(row, col).b() = clamp(255. * specular(row, col).b(), 0., 255.);
      texture4(row, col).a() = clamp(255. * shininess(row, col), 0., 255.);
    }
  }
  return texture4;
}

Image<RGB> const& 
MaterialMap::
getEmissionTexture()
{
  #pragma omp for
  for (int row=0; row<texture3.getRows(); ++row)
  {
    for (int col=0; col<texture3.getCols(); ++col)
    {
      texture3(row, col).r() = clamp(255. * emission(row, col).r(), 0., 255.);
      texture3(row, col).g() = clamp(255. * emission(row, col).g(), 0., 255.);
      texture3(row, col).b() = clamp(255. * emission(row, col).b(), 0., 255.);
    }
  }
  return texture3;
}

template <typename Pixel_t>
void
MaterialMap::
AddCairoMaterialComponent(Image<Pixel_t> &component,
                          cairo_surface_t *cSurface,
                          Pixel_t value, double transparency,
                          bool invert)
{
  int stride = cairo_image_surface_get_stride(cSurface);
  byte *argb = cairo_image_surface_get_data(cSurface);
  
  double factor = (1. - transparency) / 255.;
  #pragma omp parallel for
  for (int row=0; row<component.getRows(); ++row)
  {
    byte *pargb = argb + row*stride + 1; // point to red channel
    for (int col=0; col<component.getCols(); ++col)
    {
      if (!*pargb ^ !invert)
      {
        double alpha;
        if (invert)
        {
          alpha = factor * (255 - *pargb);
        } else {
          alpha = factor * *pargb;
        }
        component(row, col) =
          (1. - alpha) * component(row, col) + alpha * value;
      }
      pargb += 4;
    }
  }
}

