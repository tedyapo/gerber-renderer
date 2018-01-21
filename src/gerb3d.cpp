/*
  to-do: 
[x] interpolated normals at each vertex on holes
[x] per-pixel shader with bump mapping
[ ] simple shader w/simple texture
[ ] non-shader mode with simple texture
    -> most features used are deprecated in modern opengls'
[ ] change to using VBO's for all geometry for future-compatibility 
[ ] parsers for all files
[ ] glui prototype
[ ] glew prototype
[ ] use same shader for holes as edges: simple non-textured, non-bump mapped
[ ] get all winding orders and normal orientations correct
[ ] add front-and-back rendering: shader should also light back-faces
[x] note: 1oz copper = 1.37mils
[ ] modify texture gen and texture coords to use square pixels 
    -> ignore part of texture map for non-square boards
[ ] nominal soldermask thickness = 1.2mils
[x] pads vs traces in texture maps
    -> use combination of copper and mask layers to generate
[ ] get units correct for bump mapping normal calculation
[ ] allow exaggerated bump mapping mode for better visualization
    -> allow 1x-100x height? (10x?)
[ ] add emission components to Material, json files, parser, and shaders
[ ] add neon-orange material as default: use for uninitialized values
[ ] implement height setting from process file through to bump map gen
[ ] investigate linear light sources: can this be done analytically?
    -> maybe finding the maximum point on line for specular term
    -> investigate diffuse term for linear lights
    -> minimization looks difficult; maybe a geometric trick
    [ ] try using maxima to minimize; throw vectors at it, too
 */
/*
ideas for UI:

layers interface: save as board JSON file

"layer name"  "dropdown for type" "checkbox for visible"

process interface: save as process file

material interface: save as material file

 */
/*
to-do:

[ ] incorporate GLEW to probe extensions and capabilities
[ ] incorporate GLUI for user interface
[ ] incorporate jsoncpp to read JSON init files
    -> standard set of stock init files, plus read from user version(s)
[ ] add bump mapping and per-pixel material properties using glsl
    -> add normal map for bumps (one texture)
    -> compute bump map based on copper, mask, and silk thicknesses
    -> use one texture for material ambient color, plus transparency
    -> use one texture for material diffuse color, plus transparency
    -> use one texture for material specular color, plus exponent
    -> how to avoid texture for holes?
[ ] add metallic brdfs with glsl for pads, traces
[ ] allow non-lighting mode
[ ] allow basic lighting mode
[ ] UI
    -> GLUI spinner for light source (2x? 3x?)
    -> menus for enable/disable
        [ ] front
        [ ] back
        [ ] individual layer selections (e.g. copper + pads)
        [ ] holes
    -> open file(s), save as JSON set
[ ] incorporate zlib (+others) to parse zip files of gerbers? would be nice
[ ] auto-detect (guess) layer type from filename using common conventions
[ ] allow wireframe mode?
[ ] figure out how to do internal routing: how is this communicated via gerbers
[ ] allow to highlight a single net in copper???
[ ] mask color for tented vias?
[ ] allow to reload files; load files singly
[ ] need to keep gerbv project around for recalculations
[ ] wrap gerbv project in a class to provide support for:
    -> load/reload of gerber files
    -> calculation/re-calculation of texture files, bump maps, etc
[ ] cross-platform build system
[ ] get into git
[ ] get on github
[ ] try using cairo to mix layers - is it faster than mixlayer()?
[ ] allow to view layers without substrate
*/

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
#include "Process.hpp"
#include "BoardLayer.hpp"
#include "Board.hpp"
#include "json_parsing.hpp"

/*
static void
CheckError(const char *file, int line){
   GLenum err = glGetError();
   if (err) {
     printf("%s (%d): GL Error %s (0x%x)\n",
	    file, line, gluErrorString(err), (int) err);
   }
}

#define CHECK_ERROR CheckError(__FILE__, __LINE__);
*/

class glWindow {
public:

  glWindow(Drawable *drawable){
    this->drawable = drawable;
  }

  void Open(){
    close_request = false;
    pthread_mutex_init(&rendering_mutex, NULL);
    pthread_create(&rendering_thread, NULL, Open_, NULL);
  }

  void Close(){
    close_request = true;
    Wait();
    pthread_mutex_destroy(&rendering_mutex);
  }

  // waits for rendering thread to quit, then returns
  void Wait(){
    void *status;
    pthread_join(rendering_thread, &status);
  }

private:
  static Drawable *drawable;
  pthread_t rendering_thread;
  pthread_mutex_t rendering_mutex;
  bool close_request;

  static void display(){
    drawable->Display();
  }

  static void keyboard(unsigned char key, int x, int y){
    drawable->Keyboard(key, x, y);
  }

  static void *Open_(void *t){
    int argc = 1;
    char const *args = "none";
    char const **argv = &args;
    glutInit(&argc, (char**)argv);
    //glutInitDisplayString("rgba=8 depth=24 double samples=4");
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_SRGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("gerb3vd - soroban_main");
//void glutSetWindowTitle(char *name);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glEnable(GL_MULTISAMPLE_ARB);
    zprInit();
    drawable->Init();
    glutMainLoop();
    return NULL;
  }
};
Drawable* glWindow::drawable;




// find maximum texture size supported
// this doesn't seem to work (maybe need to try individual textures?? 
int
MaxTextureSize()
{
  for (int size = 8192; size > 128; size /= 2){
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, size, size,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
    GLint width; 
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    if (width == size){
      return size;
    }
  }
  return 0;
}

            /*
get world coords of camera for depth sorting sides and passing to shader
glGetIntegerv(GL_VIEWPORT, ar)
void glGetDoublev(GLenum  pname,  GLdouble *  params);
GL_MODELVIEW_MATRIX
GL_PROJECTION_MATRIX
GL_VIEWPORT
gluUnProject( 
  (viewport[2]-viewport[0])/2 , (viewport[3]-viewport[1])/2, 
  0.0,  
  model, proj, view,  
  &cx,&cy,&cz 
)
            */

int main(int argc, char **argv){
  Board board;

  const char *filename = "material.json";
  std::vector<Material> materials;
  ParseMaterial(filename, materials);
  
  std::cout << "Materials : " << materials.size() << std::endl;

  filename = "process.json";
  std::vector<Process> processes;
  ParseProcess(filename, processes, materials);

  std::cout << "Processes : " << processes.size() << std::endl;

  if (argc == 1)
  {
    filename = "soroban_clock.json";
  } else {
    filename = argv[1];
  }
  ParseBoard(filename, board, processes);

  glWindow window(&board);
  window.Open();
  window.Wait();
}
