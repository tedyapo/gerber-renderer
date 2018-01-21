#ifndef BOARD_HPP_INCLUDED_
#define BOARD_HPP_INCLUDED_

#define GL_GLEXT_PROTOTYPES
#define GL_API
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <pthread.h>
#include <unistd.h>
#include "zpr.h"
#include "Image.h"
#include "gerbv.h"
#include <iostream>
#include <cmath>
#include "VectorN.hpp"
#include <vector>
#include <string>
#include "glslshader.hpp"

#include "Material.hpp"
#include "BoardLayer.hpp"
#include "Process.hpp"

const double PI = 3.14159265358979323843;

static GLfloat light_ambient[]  = { 0.35, 0.35, 0.35, 1.0 };
static GLfloat light_diffuse[]  = { 1., 1., 1., 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//static GLfloat light_position[] = { -10, -20, 1.0, 1.0 };
static GLfloat light_position[] = { 1, 1, 1.0, 1.0 };
//static GLfloat light_position[] = { 10, 20, 10.0, 1.0 };
//static GLfloat light_position[] = { 10, 20, 10.0, 1.0 };
static GLfloat light_position2[] = { 10, 20, 20.0, 1.0 };
static GLfloat light_position3[] = { 10, 20, 40.0, 1.0 };
static GLfloat light_position4[] = { 0, 0, 10.0, 1.0 };


static GLfloat mat_ambient[]    = { 0.3, 0.3, 0.3, 1.0 };
static GLfloat mat_diffuse[]    = { 0.5, 0.5, 0.5, 1.0 };
static GLfloat mat_specular[]   = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat high_shininess[] = { 100.0 };

class Drawable {
public:
  virtual void Init(){};
  virtual void Display() = 0;
  virtual void Keyboard(unsigned char key, int x, int y){}
  virtual void Special(){}
};

class Board: public Drawable {
public:
  Board(){
    holes_list = 0;
    thickness = 1./16.;
    hole_facets = 31;
    texture_size = 4096;
    //texture_size = 8192;
    //background = {127, 127, 127, 0};
    background = {21, 21, 21, 0};

#if 0
    substrate = {31, 31, 31, 255};
    top_pads = {212, 175, 55, 255};
    top_mask = {50, 0, 50, 220};
    top_silk = {255, 255, 255, 255};
    bottom_pads = {212, 175, 55, 255};
    bottom_mask = {50, 0, 50, 220};
    bottom_silk = {255, 255, 255, 255};
#endif

    render_top_face = true;
    render_bottom_face = true;
    render_edges = true;
    render_holes = true;
    render_top_copper = true;
    render_top_mask = true;
    render_top_silk = true;
    render_bottom_copper = true;
    render_bottom_mask = true;
    render_bottom_silk = true;

    use_shader = true;
  }

  ~Board(){
    // !!! check if this has been allocated yet
//    glDeleteLists(holes_list, 1);
  }
  
  void AddLayer(std::string const& filename, BoardLayer::layer_type type);

  void Init(){

    glEnable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); 
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); 
    glEnable(GL_POLYGON_SMOOTH);
    glClearColor(background.r()/255., background.g()/255.,
                 background.b()/255., background.a()/255.);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);

    glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position3);

    glLightfv(GL_LIGHT3, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT3, GL_POSITION, light_position4);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);


    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
//    glEnable(GL_COLOR_MATERIAL);
    //  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);

    if (use_shader)
    {
      face_shader = new GLSLShader("face.vs", "face.fs");
//      face_shader = new GLSLShader("face.vs", "linear_face.fs");
      hole_shader = new GLSLShader("hole.vs", "hole.fs");
    }

    this->CreateTextures();
  }


  void GenerateHoles(){
    holes_list = glGenLists(1);
    glNewList(holes_list, GL_COMPILE);
    glEndList();
  }

  void Display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double d = this->thickness / 2.;

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // !!! change front, back rendering order based on modelview matrix
    // use normal dotted with view vector to determine orientation
    // make two passes: draw all back-facing surfaces, then all front

    if (render_holes){
      hole_shader->Enable();
      this->process.getTopPadMaterial().setGLMaterial();

      glCallList(holes_list);
    }

    if (render_top_face){
      // face_shader->Enable();

      face_shader->Enable();
      face_shader->SetVariable("normal_tex", 0);
      face_shader->SetVariable("ambient_tex", 1);
      face_shader->SetVariable("diffuse_tex", 2);
      face_shader->SetVariable("specular_tex", 3);
      // face_shader->Disable();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture( GL_TEXTURE_2D, top_normal );
      glActiveTexture(GL_TEXTURE1);
      glBindTexture( GL_TEXTURE_2D, top_ambient );
      glActiveTexture(GL_TEXTURE2);
      glBindTexture( GL_TEXTURE_2D, top_diffuse );
      glActiveTexture(GL_TEXTURE3);
      glBindTexture( GL_TEXTURE_2D, top_specular );

      glBegin(GL_QUADS);
      glNormal3d(0., 0., 1.);
      glTexCoord2d(1., 1.);
      glVertex3d(tr_corner.x(), bl_corner.y(), +d);
      glTexCoord2d(1., 0.);
      glVertex3d(tr_corner.x(), tr_corner.y(), +d);
      glTexCoord2d(0., 0.);
      glVertex3d(bl_corner.x(), tr_corner.y(), +d);
      glTexCoord2d(0., 1.);
      glVertex3d(bl_corner.x(), bl_corner.y(), +d);
      glEnd();
    }

    if (render_bottom_face){
      face_shader->Enable();

      face_shader->SetVariable("normal_tex", 4);
      face_shader->SetVariable("ambient_tex", 5);
      face_shader->SetVariable("diffuse_tex", 6);
      face_shader->SetVariable("specular_tex", 7);

/*
      glActiveTexture(GL_TEXTURE0);
      glBindTexture( GL_TEXTURE_2D, bottom_normal );
      glActiveTexture(GL_TEXTURE1);
      glBindTexture( GL_TEXTURE_2D, bottom_ambient );
      glActiveTexture(GL_TEXTURE2);
      glBindTexture( GL_TEXTURE_2D, bottom_diffuse );
      glActiveTexture(GL_TEXTURE3);
      glBindTexture( GL_TEXTURE_2D, bottom_specular );
*/

      glBegin(GL_QUADS);
      glNormal3d(0., 0., -1.);
      glTexCoord2d(0., 1.);
      glVertex3d(bl_corner.x(), bl_corner.y(), -d);
      glTexCoord2d(0., 0.);
      glVertex3d(bl_corner.x(), tr_corner.y(), -d);
      glTexCoord2d(1., 0.);
      glVertex3d(tr_corner.x(), tr_corner.y(), -d);
      glTexCoord2d(1., 1.);
      glVertex3d(tr_corner.x(), bl_corner.y(), -d);
      glEnd();
    }

    if (render_edges){
      if (use_shader){
        hole_shader->Enable();
      }
      glDisable( GL_TEXTURE_2D );
      this->process.getSubstrateMaterial().setGLColor();
/*
!!!
      need normals here; why is color rendering incorrectly
check winding order also
clean up bl_ tr_ corner mess
*/

      glBegin(GL_QUADS);
      glVertex3d(bl_corner.x(), bl_corner.y(), +d);
      glVertex3d(bl_corner.x(), bl_corner.y(), -d);
      glVertex3d(bl_corner.x(), tr_corner.y(), -d);
      glVertex3d(bl_corner.x(), tr_corner.y(), +d);
      
      glVertex3d(bl_corner.x(), bl_corner.y(), +d);
      glVertex3d(bl_corner.x(), bl_corner.y(), -d);
      glVertex3d(tr_corner.x(), bl_corner.y(), -d);
      glVertex3d(tr_corner.x(), bl_corner.y(), +d);
      
      glVertex3d(tr_corner.x(), tr_corner.y(), +d);
      glVertex3d(tr_corner.x(), tr_corner.y(), -d);
      glVertex3d(tr_corner.x(), bl_corner.y(), -d);
      glVertex3d(tr_corner.x(), bl_corner.y(), +d);
      
      glVertex3d(tr_corner.x(), tr_corner.y(), +d);
      glVertex3d(tr_corner.x(), tr_corner.y(), -d);
      glVertex3d(bl_corner.x(), tr_corner.y(), -d);
      glVertex3d(bl_corner.x(), tr_corner.y(), +d);
      glEnd();
    }

    glutSwapBuffers();
  }

  void Keyboard(unsigned char key, int x, int y){
    if (key == 'q'){
      exit(0);
    }
    glutPostRedisplay();
  }

  std::string const& getName() const
  {
    return name;
  }
  void setName(std::string const& new_name)
  {
    this->name = new_name;
  }
  void setProcess(Process const& new_process)
  {
    this->process = new_process;
  }
private:
  std::string name;
  GLSLShader *face_shader;
  GLSLShader *hole_shader;
  bool use_shader;
  std::vector<BoardLayer> layers;
  Process process;
  GLuint top_texture;

  // encapsulate these texture map ids for material textures?
  GLuint top_normal;
  GLuint top_ambient;
  GLuint top_diffuse;
  GLuint top_specular;

  GLuint bottom_normal;
  GLuint bottom_ambient;
  GLuint bottom_diffuse;
  GLuint bottom_specular;

  GLuint bottom_texture;

  GLuint holes_list;
  v3d offset;
  v3d bl_corner;
  v3d tr_corner;
  double thickness;
  int hole_facets;
  int texture_size;
  RGBA background;

  bool render_top_face;
  bool render_bottom_face;
  bool render_edges;
  bool render_holes;
  bool render_top_copper;
  bool render_top_mask;
  bool render_top_silk;
  bool render_bottom_copper;
  bool render_bottom_mask;
  bool render_bottom_silk;

  void CreateTextures();
  void DrawHole(double xc, double yc, double r);
  void DrawHoles(gerbv_image_t *inputImage);
  void RenderLayer(gerbv_project_t *mainProject, cairo_t *cairoTarget,
                   gerbv_render_info_t *renderInfo,
                   BoardLayer::layer_type layer_type);
  void MixLayer(unsigned char *rgba, unsigned char *argb,
                int rows, int cols, int stride, RGBA color, bool invert,
                bool mix_alpha = false);

};


#endif // #ifndef BOARD_HPP_INCLUDED_
