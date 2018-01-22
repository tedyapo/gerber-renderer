#include "json_parsing.hpp"
#include <json-c/json.h>
#include <string>
#include <sstream>
#include <cerrno>

v3d
ParseColor3(json_object* material_obj, const char* element)
{
  v3d color;
  struct json_object *color_obj;
  struct json_object *component_obj;

  color_obj = json_object_object_get(material_obj, element);
  component_obj = json_object_object_get(color_obj, "red");
  color.r() = json_object_get_double(component_obj);
  component_obj = json_object_object_get(color_obj, "green");
  color.g() = json_object_get_double(component_obj);
  component_obj = json_object_object_get(color_obj, "blue");
  color.b() = json_object_get_double(component_obj);

  return color;
}

v4d
ParseColor4(json_object* material_obj, const char* element)
{
  v4d color;
  struct json_object *color_obj;
  struct json_object *component_obj;

  color_obj = json_object_object_get(material_obj, element);
  component_obj = json_object_object_get(color_obj, "red");
  color.r() = json_object_get_double(component_obj);
  component_obj = json_object_object_get(color_obj, "green");
  color.g() = json_object_get_double(component_obj);
  component_obj = json_object_object_get(color_obj, "blue");
  color.b() = json_object_get_double(component_obj);
  component_obj = json_object_object_get(color_obj, "alpha");
  color.a() = json_object_get_double(component_obj);

  return color;
}

/* re-write below */
int get_file_contents(const char* filename, char** outbuffer);

void
ParseMaterial(const char *filename, std::vector<Material> &materials)
{
  Material material;
  struct json_object *root_obj;
  struct json_object *materials_obj;
  struct json_object *material_obj;
  struct json_object *name_obj;
  struct json_object *aliases_obj;
  struct json_object *alias_obj;
  struct json_object *value_obj;
  const char *name;
  char *buffer = NULL;

  get_file_contents(filename, &buffer);
  root_obj = json_tokener_parse(buffer);
  
  materials_obj = json_object_object_get(root_obj, "materials");
  int materials_len = json_object_array_length(materials_obj);
  for (int i = 0; i < materials_len; ++i) {
    material_obj = json_object_array_get_idx(materials_obj, i);
    name_obj = json_object_object_get(material_obj, "name");
    name = json_object_get_string(name_obj);
    material.setName(name);
    material.addAlias(name);
    
    aliases_obj = json_object_object_get(material_obj, "aliases");
    if (NULL != aliases_obj)
    {
      int aliases_len = json_object_array_length(aliases_obj);
      for (int i = 0; i < aliases_len; ++i) {
        alias_obj = json_object_array_get_idx(aliases_obj, i);
        name = json_object_get_string(alias_obj);
        material.addAlias(name);
      }
    }

    material.setColor(ParseColor4(material_obj, "color"));
    material.setAmbient(ParseColor3(material_obj, "ambient"));
    material.setDiffuse(ParseColor3(material_obj, "diffuse"));
    material.setSpecular(ParseColor3(material_obj, "specular"));
    value_obj = json_object_object_get(material_obj, "shininess");
    material.setShininess(json_object_get_double(value_obj));
    value_obj = json_object_object_get(material_obj, "transparency");
    material.setTransparency(json_object_get_double(value_obj));

    materials.push_back(material);
  }
  free(buffer);
}

//
// convert string containing units of linear measure to inches
//
double
UnitsToInches(const char *value_units)
{
  std::stringstream str(value_units);
  double value;
  std::string units;

  str >> value;
  str >> units;

  // switch-like construct for string compares
  //  !!! why not just return result instead of using break here?
  do
  {
    if (units == "mil" ||
        units == "mils")
    {
      value *= 0.001;
      break;
    }
    if (units == "in" ||
        units == "inch" ||
        units == "inches" ||
        units == "\"")
    {
      value *= 1.;
      break;
    }
    if (units == "um" ||
        units == "micrometer" ||
        units == "micron")
    {
      value *= 0.0000393700787;
      break;
    }
    if (units == "mm" ||
        units == "millimeter" ||
        units == "millimeters")
    {
      value *= 0.0393700787;
      break;
    }
    if (units == "cm" ||
        units == "centimeter" ||
        units == "centimeters")
    {
      value *= 0.393700787;
      break;
    }
    // allow thicknesses in oz of Cu
    if (units == "oz" ||
        units == "ounces")
    {
      value *= 0.00137;
      break;
    }

    // !!! raise a warning about unknown or unspecified units
  } while (0);

  return value;
}

double
GetJSONValueUnits(struct json_object *process_obj, const char *name)
{
  struct json_object *value_obj;
  const char *value_string;

  value_obj = json_object_object_get(process_obj, name);
  value_string = json_object_get_string(value_obj);
  return UnitsToInches(value_string);
}

Material const&
GetJSONMaterial(struct json_object *process_obj,
                std::string const& name,
                std::vector<Material> const& materials)
{
  struct json_object *value_obj;
  const char *value_string;

  value_obj = json_object_object_get(process_obj, name.c_str());
  value_string = json_object_get_string(value_obj);

  std::cout << "looking up material (" << value_string << ")" << std:: endl;
  for (unsigned i=0; i<materials.size(); ++i)
  {
    if (materials[i].isAlias(value_string))
    {
      return materials[i];
    }
  }
  return Material::DefaultMaterial();
}

void
ParseProcess(const char *filename, std::vector<Process> &processes,
                  std::vector<Material> const& materials)
{
  Process process;
  struct json_object *root_obj;
  struct json_object *processes_obj;
  struct json_object *process_obj;
  struct json_object *name_obj;
  const char *name;
  char *buffer = NULL;

  get_file_contents(filename, &buffer);

  root_obj = json_tokener_parse(buffer);
  processes_obj = json_object_object_get(root_obj, "processes");
  int processes_len = json_object_array_length(processes_obj);
  for (int i = 0; i < processes_len; ++i) {
    process_obj = json_object_array_get_idx(processes_obj, i);
    name_obj = json_object_object_get(process_obj, "name");
    name = json_object_get_string(name_obj);
    process.setName(name);

    process.setSubstrateMaterial(GetJSONMaterial(process_obj,
                                                 "substrate_material",
                                                 materials));
    process.setSubstrateThickness(GetJSONValueUnits(process_obj,
                                                    "substrate_thickness"));

    process.setTopPadMaterial(GetJSONMaterial(process_obj,
                                              "top_pad_material",
                                              materials));
    process.setTopTraceMaterial(GetJSONMaterial(process_obj,
                                                "top_trace_material",
                                                materials));
    process.setTopCopperThickness(GetJSONValueUnits(process_obj,
                                                    "top_copper_thickness"));

    process.setTopMaskMaterial(GetJSONMaterial(process_obj,
                                               "top_mask_material",
                                               materials));
    process.setTopMaskThickness(GetJSONValueUnits(process_obj,
                                                  "top_mask_thickness"));

    process.setTopSilkMaterial(GetJSONMaterial(process_obj,
                                               "top_silk_material",
                                               materials));
    process.setTopSilkThickness(GetJSONValueUnits(process_obj,
                                                  "top_silk_thickness"));

    process.setBottomPadMaterial(GetJSONMaterial(process_obj,
                                                 "bottom_pad_material",
                                                 materials));
    process.setBottomTraceMaterial(GetJSONMaterial(process_obj,
                                                   "bottom_trace_material",
                                                   materials));
    process.setBottomCopperThickness(GetJSONValueUnits(process_obj,
                                                       "bottom_copper_thickness"));
    process.setBottomMaskMaterial(GetJSONMaterial(process_obj,
                                                  "bottom_mask_material",
                                                  materials));
    process.setBottomMaskThickness(GetJSONValueUnits(process_obj,
                                                     "bottom_mask_thickness"));

    process.setBottomSilkMaterial(GetJSONMaterial(process_obj,
                                                  "bottom_silk_material",
                                                  materials));
    process.setBottomSilkThickness(GetJSONValueUnits(process_obj,
                                                     "bottom_silk_thickness"));

    processes.push_back(process);
  }
  free(buffer);
}

std::string
GetJSONValue(struct json_object *base_obj, const char *key)
{
  struct json_object *value_obj;
  value_obj = json_object_object_get(base_obj, key);
  std::string str = json_object_get_string(value_obj);
  return str;
}

Process const&
lookupProcess(std::vector<Process> const& processes, std::string process_name)
{
  for (unsigned i=0; i<processes.size(); ++i)
  {
    if (processes[i].getName() == process_name)
    {
      return processes[i];
    }
  }
  return Process::DefaultProcess();
}

void
ParseBoard(const char *filename, Board &board,
           std::vector<Process> const& processes)
{
  struct json_object *root_obj;
  struct json_object *files_obj;
  struct json_object *file_obj;
  struct json_object *name_obj;
  struct json_object *value_obj;
  char *buffer = NULL;

  get_file_contents(filename, &buffer);
  root_obj = json_tokener_parse(buffer);
  
  std::string name = GetJSONValue(root_obj, "name");
  board.setName(name);

  std::string directory = GetJSONValue(root_obj, "directory");

  files_obj = json_object_object_get(root_obj, "files");
  if (NULL != files_obj)
  {
    int files_len = json_object_array_length(files_obj);
    for (int i = 0; i < files_len; ++i) {
      file_obj = json_object_array_get_idx(files_obj, i);
      std::string filename = GetJSONValue(file_obj, "filename");
      std::string filetype = GetJSONValue(file_obj, "type");
      BoardLayer::layer_type type = BoardLayer::lookupLayerType(filetype);
      if (directory.back() == '/')
      {
        board.AddLayer(directory + filename, type);
      } else {
        board.AddLayer(directory + "/" + filename, type);
      }
    }
  }

  std::string process_name = GetJSONValue(root_obj, "process");
  board.setProcess(lookupProcess(processes, process_name));

  free(buffer);
}


///////!!!!! rewrite this in c++; avoid copyright from json-c code
/**
 * Place the contents of the specified file into a memory buffer
 *
 * @param[in] filename The path and name of the file to read
 * @param[out] filebuffer A pointer to the contents in memory
 * @return status 0 success, 1 on failure
 */
int get_file_contents(const char* filename, char** outbuffer) {
  FILE* file = NULL;
  long filesize;
  const int blocksize = 1;
  size_t readsize;
  char* filebuffer;

  // Open the file
  file = fopen(filename, "r");
  if (NULL == file)
  {
    printf("'%s' not opened\n", filename);
    exit(EXIT_FAILURE);
  }

  // Determine the file size
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  rewind (file);

  // Allocate memory for the file contents
  filebuffer = (char*) malloc(sizeof(char) * filesize);
  *outbuffer = filebuffer;
  if (filebuffer == NULL)
  {
    fputs ("malloc out-of-memory", stderr);
    exit(EXIT_FAILURE);
  }

  // Read in the file
  readsize = fread(filebuffer, blocksize, filesize, file);
  if (readsize != filesize)
  {
    fputs ("didn't read file completely",stderr);
    exit(EXIT_FAILURE);
  }

  // Clean exit
  fclose(file);
  return EXIT_SUCCESS;
}

