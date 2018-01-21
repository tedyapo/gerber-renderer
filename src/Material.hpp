#ifndef MATERIAL_HPP_INCLUDED_
#define MATERIAL_HPP_INCLUDED_

#include <vector>
#include "VectorN.hpp"
#include "Material.hpp"
#include <string>
#define GL_GLEXT_PROTOTYPES
#define GL_API
#include <GL/gl.h>

// opengl material encapsulation
class Material
{
public:
  std::string const& getName() const { return name; }
  v4d getColor() const {return color;}
  v3d getAmbient() const {return ambient;}
  v3d getDiffuse() const {return diffuse;}
  v3d getSpecular() const {return specular;}
  v3d getEmission() const {return emission;}
  double getShininess() const {return shininess;}
  double getTransparency() const {return transparency;}

  void setName(std::string newval)
  {
    name = newval;
  }
  void addAlias(std::string newval)
  {
    aliases.push_back(newval);
  }
  void setColor(v4d newval)
  {
    color = newval;
  }
  void setAmbient(v3d newval)
  {
    ambient = newval;
  }
  void setDiffuse(v3d newval)
  {
    diffuse = newval;
  }
  void setSpecular(v3d newval)
  {
    specular = newval;
  }
  void setEmission(v3d newval)
  {
    emission = newval;
  }
  void setShininess(double newval)
  {
    shininess = newval;
  }
  void setTransparency(double newval)
  {
    transparency = newval;
  }

  void setGLColor() const
  {
    glColor4d(color.r(), color.g(), color.b(), color.a());
  }

  void setGLMaterial() const
  {
    float v[4];
    v[3] = 0.f;
    for (int i=0; i<3; ++i) { v[i] = ambient(i); }
    glMaterialfv(GL_FRONT, GL_AMBIENT, v);

    for (int i=0; i<3; ++i) { v[i] = specular(i); }
    glMaterialfv(GL_FRONT, GL_SPECULAR, v);

    for (int i=0; i<3; ++i) { v[i] = diffuse(i); }
    v[3] = 1. - transparency;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, v);

    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  }

  bool isAlias(const char *name) const
  {
    if (this->name == name)
    {
      return true;
    }
    for (unsigned i=0; i<aliases.size(); ++i)
    {
      if (aliases[i] == name)
      {
        return true;
      }
    }
    return false;
  }

  static Material const& DefaultMaterial()
  {
    Material material = Material();
    default_material = material;
    return default_material;
  }
private:
  std::string name;
  std::vector<std::string> aliases;
  v4d color;  // basic RGBA color for color-only rendering
  v3d ambient;
  v3d diffuse;
  v3d specular;
  v3d emission;
  double shininess;
  double transparency;
  static Material default_material;
};



#endif // #ifndef MATERIAL_HPP_INCLUDED_
