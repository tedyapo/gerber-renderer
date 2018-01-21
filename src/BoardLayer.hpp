#ifndef BOARDLAYER_INCLUDED_
#define BOARDLAYER_INCLUDED_

class BoardLayer
{
public:
  typedef enum {LAYER_UNKNOWN, LAYER_OUTLINE, LAYER_DRILL,
                LAYER_TOP_COPPER, LAYER_TOP_MASK, LAYER_TOP_SILK,
                LAYER_BOTTOM_COPPER, LAYER_BOTTOM_MASK, LAYER_BOTTOM_SILK,
                LAYER_MILL}
    layer_type;

  BoardLayer(const std::string &filename, layer_type type,
             Material const& material, bool render_flag){
    this->filename = filename;
    this->type = type;
    this->material = material;
    this->render_flag = render_flag;
  }
  std::string getFilename() { return filename; }
  layer_type getType() { return type; }
  Material const& getMaterial() { return material; }
  bool getRenderFlag() { return render_flag; }
  static layer_type lookupLayerType(std::string const& name)
  {
    if (name == "outline")
    {
      return LAYER_OUTLINE;
    }
    if (name == "holes")
    {
      return LAYER_DRILL;
    }
    if (name == "top_copper")
    {
      return LAYER_TOP_COPPER;
    }
    if (name == "top_mask")
    {
      return LAYER_TOP_MASK;
    }
    if (name == "top_silk")
    {
      return LAYER_TOP_SILK;
    }
    if (name == "bottom_copper")
    {
      return LAYER_BOTTOM_COPPER;
    }
    if (name == "bottom_mask")
    {
      return LAYER_BOTTOM_MASK;
    }
    if (name == "bottom_silk")
    {
      return LAYER_BOTTOM_SILK;
    }
    if (name == "mill")
    {
      return LAYER_MILL;
    }
    return LAYER_UNKNOWN;
  }
private:
  layer_type type;
  std::string filename;
  Material material;
  bool render_flag;
};

#endif // #ifndef BOARDLAYER_INCLUDED_
