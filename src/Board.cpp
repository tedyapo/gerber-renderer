#include "Board.hpp"
#include "BumpMap.hpp"
#include "MaterialMap.hpp"
#include "ImageOps.h"

/*
 to-do:
[ ] trace edges should be scaler correctly
    -> now, edge width depends on texture map resolution (raw pixel units)
    -> ideally, begin with cairo surface, then blur with simple box filter
    -> size of box filter depends on texture map resolution
[ ] encapsulate face or layer so top and bottom can be treated similarly
    for rendering
[ ] encapsulate texture setting and texture units to communicate with
    shader(s)
[ ] consider compositing layers in opengl for speed
    -> may be able to store raw cairo-rendered images on GPU and
       composite as required; this would be very fast
*/

// !!! hole_facets should be configurable, and may depend on hole diameter
void
Board:: 
DrawHole(double xc, double yc, double diameter){
  double d = this->thickness / 2.;
  double r = diameter / 2.;
  double oldx = xc + r;
  double oldy = yc;
  int N = this->hole_facets * r / 0.02;

  for (int i=1; i<=N; ++i){
    double theta = 2*PI*i/N;
    double x = xc + r * std::cos(theta);
    double y = yc + r * std::sin(theta);

//    v3d n = v3d(x-oldx, y-oldy, 0.).cross(v3d(x-oldx, y-oldy, -2.*d));
//    n.normalize();
//    glNormal3d(n.x(), n.y(), n.z());
    glBegin(GL_QUADS);
    v3d n = v3d(xc - oldx, yc - oldy, 0.);
    n.normalize();
    glNormal3d(n.x(), n.y(), n.z());
    glVertex3d(oldx, oldy, d);
    glVertex3d(oldx, oldy, -d);

    n = v3d(xc - x, yc - y, 0.);
    n.normalize();
    glNormal3d(n.x(), n.y(), n.z());
    glVertex3d(x, y, -d);
    glVertex3d(x, y, d);
    glEnd();

    oldx = x;
    oldy = y;
  }
}

void 
Board::
DrawHoles(gerbv_image_t *inputImage){
  // no-op transform
  gerbv_user_transformation_t transform = {0., 0.,
                                           1., 1.,
                                           FALSE};
	
	/* duplicate the image, cleaning it in the process */
	gerbv_image_t *image = gerbv_image_duplicate_image (inputImage, &transform);

  /* run through all the nets in the layer, rendering holes */
  for (gerbv_net_t *currentNet = image->netlist; currentNet;
       currentNet = currentNet->next){
    if ((currentNet->aperture_state != GERBV_APERTURE_STATE_OFF) &&
        (image->aperture[currentNet->aperture] != NULL) &&
        (image->aperture[currentNet->aperture]->type == GERBV_APTYPE_CIRCLE)){
      double xc = currentNet->stop_x - this->offset.x();
      double yc = currentNet->stop_y - this->offset.y();
      double diameter = image->aperture[currentNet->aperture]->parameter[0];
      this->DrawHole(xc, yc, diameter);
    }
  }
  
  // destroy image copy we made
	gerbv_destroy_image (image);
}


void
Board::
AddLayer(std::string const& filename, BoardLayer::layer_type type)
{
  bool render_flag = true;
  Material const& material = this->process.getLayerMaterial(type);
  BoardLayer layer(filename, type, material, render_flag);
  this->layers.push_back(layer);
}

void
Board::
RenderLayer(gerbv_project_t *mainProject, cairo_t *cairoTarget,
            gerbv_render_info_t *renderInfo, BoardLayer::layer_type layer_type)
{
  // clear cairo surface
  cairo_save(cairoTarget);
  cairo_set_operator(cairoTarget, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cairoTarget);
  cairo_restore(cairoTarget);

  // render layer(s)
  for (unsigned i=0; i<this->layers.size(); ++i){
    if (this->layers[i].getType() == layer_type){
      cairo_save(cairoTarget);
      gerbv_render_layer_to_cairo_target (cairoTarget,
                                          mainProject->file[i],
                                          renderInfo);
      cairo_restore(cairoTarget);
      cairo_surface_flush(cairo_get_target(cairoTarget));
    }
  }
}

/*
to-do

[ ] bump map object
    -> allow sending to opengl??
[ ] material map object
    -> handle transparency better
    -> allow sending to opengl??
[ ] finish maps for bottom board face
 */

template <typename pixel_t>
struct PixelGLTraits
{
  // no defaults
};

template <>
struct PixelGLTraits<RGB> 
{
  enum {PIXT_GL_FORMAT = GL_RGB};
  enum {PIXT_GL_TYPE = GL_UNSIGNED_BYTE};
};

template <>
struct PixelGLTraits<RGBA> 
{
  enum {PIXT_GL_FORMAT = GL_RGBA};
  enum {PIXT_GL_TYPE = GL_UNSIGNED_BYTE};
};

//
// !!! update this with the new way of creating mipmaps (see web)
//
template <typename pixel_t>
void 
BuildTexture(GLint unit, GLuint *id, Image<pixel_t> const& image)
{
  glActiveTexture(unit);
  glEnable( GL_TEXTURE_2D );
  glGenTextures( 1, id );
  glBindTexture( GL_TEXTURE_2D, *id );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR_MIPMAP_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  gluBuild2DMipmaps( GL_TEXTURE_2D, PixelGLTraits<pixel_t>::PIXT_GL_FORMAT,
                     image.getCols(), image.getRows(),
                     PixelGLTraits<pixel_t>::PIXT_GL_FORMAT,
                     PixelGLTraits<pixel_t>::PIXT_GL_TYPE, 
                     image.getData() );
}


void
Board::
CreateTextures(){
	/* create a top level libgerbv structure */
	gerbv_project_t *mainProject = gerbv_create_project();

  // open layers and check for error
	GdkColor white = {0, 65535, 65535, 65535};
  for (unsigned i=0; i<this->layers.size(); ++i){
    gerbv_open_layer_from_filename (mainProject,
                                    const_cast<char*> 
                                    (this->layers[i].getFilename().c_str()) );
    if (mainProject->file[i] == NULL){
      std::cerr << "ERROR" << std::endl; // !!! fixme
      exit(0);
    }
    // change the color of the images to white, no transparency
    mainProject->file[i]->color = white;
    mainProject->file[i]->alpha = 65535;
  }

  // find boundingbox for board; set parameters accordingly
  gerbv_render_size_t boundingbox;
  gerbv_render_get_boundingbox(mainProject, &boundingbox);
  double height = boundingbox.bottom - boundingbox.top;
  double width = boundingbox.right - boundingbox.left;
  this->offset = v3d(boundingbox.left + width/2.,
                     boundingbox.top + height/2., 0.);
  this->bl_corner = v3d(-width/2., -height/2., 0.);
  this->tr_corner = v3d(+width/2., +height/2., 0.);
  double x_scale = this->texture_size / width;
  double y_scale = this->texture_size / height;
  gerbv_render_info_t renderInfo = {x_scale, y_scale,
                                    boundingbox.left, boundingbox.top,
                                    GERBV_RENDER_TYPE_CAIRO_HIGH_QUALITY,
                                    this->texture_size, this->texture_size};

	cairo_surface_t *cSurface = 
    cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                renderInfo.displayWidth,
                                renderInfo.displayHeight);
	cairo_t *cairoTarget = cairo_create (cSurface);


  int rows = cairo_image_surface_get_height(cSurface);
  int cols = cairo_image_surface_get_width(cSurface);

  // allocate material and bump maps
  MaterialMap top_material_map(rows, cols, false);
  BumpMap top_bump_map (rows, cols, 1./x_scale, 1./y_scale);

  // initialize board to substrate material
  top_material_map.Initialize(this->process.getSubstrateMaterial());

  // generate display list for rendering holes
  //  !!! delete list if it already exists here (must detect)
  holes_list = glGenLists(1);
  glNewList(holes_list, GL_COMPILE);
  for (unsigned i=0; i<this->layers.size(); ++i){
    if (this->layers[i].getType() == BoardLayer::LAYER_DRILL){
      this->DrawHoles(mainProject->file[i]->image);
    }
  }
  glEndList();

  // render drills layer(s) to use for texture transparency
  this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                    BoardLayer::LAYER_DRILL);
  // use drill image as transparency for board material
  top_material_map.setCairoTransparency(cSurface);

  // render mill layer(s) to use for texture transparency
  this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                    BoardLayer::LAYER_MILL);
  // add mill layer transparency for board material
  top_material_map.addCairoTransparency(cSurface);
      
  if (render_top_copper){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_TOP_COPPER);
    top_material_map.AddCairoMaterial(cSurface,
                                      this->process.getTopTraceMaterial(),
                                      false);
    top_bump_map.AddCairoHeightField(cSurface,
                                     this->process.getTopCopperThickness());
  }

  //
  // render top pads
  //
  if (render_top_copper){
    // render top copper layer
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_TOP_COPPER);

    cairo_surface_t *cSurface1 = 
      cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                  renderInfo.displayWidth,
                                  renderInfo.displayHeight);
    cairo_t *cairoTarget1 = cairo_create (cSurface1);

    // render top mask layer
    this->RenderLayer(mainProject, cairoTarget1, &renderInfo,
                      BoardLayer::LAYER_TOP_MASK);

    int stride = cairo_image_surface_get_stride(cSurface);
    byte *argb = cairo_image_surface_get_data(cSurface);

    // multiply copper and mask layers to get pads 
    double inv255 = 1. / 255.;
    byte *argb1 = cairo_image_surface_get_data(cSurface1);
    for (int row=0; row<rows; ++row){
      unsigned char *pargb = argb + row*stride + 1;
      unsigned char *pargb1 = argb1 + row*stride + 1;
      for (int col=0; col<cols; ++col){
        *pargb = (inv255 * *pargb) * *pargb1;
        pargb += 4;
        pargb1 += 4;
      }
    }

    // mix top pads into texture image
    top_material_map.AddCairoMaterial(cSurface,
                                      this->process.getTopPadMaterial(),
                                      false);
    cairo_destroy (cairoTarget1);
    cairo_surface_destroy (cSurface1);
  }

  if (render_top_mask){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_TOP_MASK);
    top_material_map.AddCairoMaterial(cSurface,
                                      this->process.getTopMaskMaterial(),
                                      true);
    top_bump_map.AddCairoHeightField(cSurface,
                                     this->process.getTopMaskThickness());
  }

  if (render_top_silk){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_TOP_SILK);
    top_material_map.AddCairoMaterial(cSurface,
                                      this->process.getTopSilkMaterial(),
                                      false);
    top_bump_map.AddCairoHeightField(cSurface,
                                     this->process.getTopSilkThickness());
  }

  // build top side normal map from image
  BuildTexture(GL_TEXTURE0, &top_normal, top_bump_map.getNormalMap());
  
  // build top side ambient map from image
  BuildTexture(GL_TEXTURE1, &top_ambient,
               top_material_map.getAmbientTexture());
  
  // build top side diffuse map from image
  BuildTexture(GL_TEXTURE2, &top_diffuse,
               top_material_map.getDiffuseTexture());
  
  // build top side specular map from image
  BuildTexture(GL_TEXTURE3, &top_specular,
               top_material_map.getSpecularTexture());

#if 0  
  face_shader->Enable();
  face_shader->SetVariable("normal_tex", 0);
  face_shader->SetVariable("ambient_tex", 1);
  face_shader->SetVariable("diffuse_tex", 2);
  face_shader->SetVariable("specular_tex", 3);
  face_shader->Disable();
#endif

  // -- bottom side textures --

  // allocate material and bump maps
  MaterialMap bottom_material_map(rows, cols, false);
  BumpMap bottom_bump_map (rows, cols, 1./x_scale, 1./y_scale);

  // initialize board to substrate material
  bottom_material_map.Initialize(this->process.getSubstrateMaterial());

  // render drills layer(s) to use for texture transparency
  this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                    BoardLayer::LAYER_DRILL);
  // use drill image as transparency for board material
  bottom_material_map.setCairoTransparency(cSurface);

  // render mill layer(s) to use for texture transparency
  this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                    BoardLayer::LAYER_MILL);
  // add mill layer transparency for board material
  bottom_material_map.addCairoTransparency(cSurface);
      
  if (render_bottom_copper){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_BOTTOM_COPPER);
    bottom_material_map.AddCairoMaterial(cSurface,
                                         this->process.getBottomTraceMaterial(),
                                         false);
    bottom_bump_map.AddCairoHeightField(cSurface,
                                        this->process.getBottomCopperThickness());
  }

  //
  // render bottom pads
  //
  if (render_bottom_copper){
    // render bottom copper layer
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_BOTTOM_COPPER);

    cairo_surface_t *cSurface1 = 
      cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                  renderInfo.displayWidth,
                                  renderInfo.displayHeight);
    cairo_t *cairoTarget1 = cairo_create (cSurface1);

    // render bottom mask layer
    this->RenderLayer(mainProject, cairoTarget1, &renderInfo,
                      BoardLayer::LAYER_BOTTOM_MASK);

    int stride = cairo_image_surface_get_stride(cSurface);
    byte *argb = cairo_image_surface_get_data(cSurface);

    // multiply copper and mask layers to get pads 
    double inv255 = 1. / 255.;
    byte *argb1 = cairo_image_surface_get_data(cSurface1);
    for (int row=0; row<rows; ++row){
      unsigned char *pargb = argb + row*stride + 1;
      unsigned char *pargb1 = argb1 + row*stride + 1;
      for (int col=0; col<cols; ++col){
        *pargb = (inv255 * *pargb) * *pargb1;
        pargb += 4;
        pargb1 += 4;
      }
    }

    // mix bottom pads into texture image
    bottom_material_map.AddCairoMaterial(cSurface,
                                         this->process.getBottomPadMaterial(),
                                         false);
    cairo_destroy (cairoTarget1);
    cairo_surface_destroy (cSurface1);
  }

  if (render_bottom_mask){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_BOTTOM_MASK);
    bottom_material_map.AddCairoMaterial(cSurface,
                                         this->process.getBottomMaskMaterial(),
                                         true);
    bottom_bump_map.AddCairoHeightField(cSurface,
                                        this->process.getBottomMaskThickness());
  }

  if (render_bottom_silk){
    this->RenderLayer(mainProject, cairoTarget, &renderInfo,
                      BoardLayer::LAYER_BOTTOM_SILK);
    bottom_material_map.AddCairoMaterial(cSurface,
                                         this->process.getBottomSilkMaterial(),
                                         false);
    bottom_bump_map.AddCairoHeightField(cSurface,
                                        this->process.getBottomSilkThickness());
  }

  // build bottom side normal map from image
  BuildTexture(GL_TEXTURE4, &bottom_normal, bottom_bump_map.getNormalMap());
  
  // build bottom side ambient map from image
  BuildTexture(GL_TEXTURE5, &bottom_ambient,
               bottom_material_map.getAmbientTexture());
  
  // build bottom side diffuse map from image
  BuildTexture(GL_TEXTURE6, &bottom_diffuse,
               bottom_material_map.getDiffuseTexture());
  
  // build bottom side specular map from image
  BuildTexture(GL_TEXTURE7, &bottom_specular,
               bottom_material_map.getSpecularTexture());

	cairo_destroy (cairoTarget);
	cairo_surface_destroy (cSurface);

	/* destroy the project, which will in turn destroy all child images */
	gerbv_destroy_project (mainProject);
}
