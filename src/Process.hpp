#ifndef PROCESS_HPP_INCLUDED_
#define PROCESS_HPP_INCLUDED_

#include <string>
#include "Material.hpp"
#include "BoardLayer.hpp"

class Process
{
public:
  Material const& getLayerMaterial(BoardLayer::layer_type type)
  {
    switch (type)
    {
    case BoardLayer::LAYER_OUTLINE:
      return substrate_material;
      break;
    case BoardLayer::LAYER_DRILL:
      return hole_material;
      break;
    case BoardLayer::LAYER_TOP_COPPER:
      return top_pad_material;
      break;
    case BoardLayer::LAYER_TOP_MASK:
      return top_mask_material;
      break;
    case BoardLayer::LAYER_TOP_SILK:
      return top_silk_material;
      break;
    case BoardLayer::LAYER_BOTTOM_COPPER:
      return bottom_pad_material;
      break;
    case BoardLayer::LAYER_BOTTOM_MASK:
      return bottom_mask_material;
      break;
    case BoardLayer::LAYER_BOTTOM_SILK:
      return bottom_silk_material;
      break;
    case BoardLayer::LAYER_MILL:
      return mill_material;
      break;
    default:
      return substrate_material;
    }
  }

  std::string const& getName() const
  {
    return name;
  }

  void setName(std::string const& new_value)
  {
    this->name = new_value;
  }

  Material const& getSubstrateMaterial() const
  {
    return substrate_material;
  }

  void setSubstrateMaterial(Material const& new_value)
  {
    this->substrate_material = new_value;
  }

  void setSubstrateThickness(double new_value)
  {
    this->substrate_thickness = new_value;
  }

  Material const& getTopTraceMaterial() const
  {
    return top_trace_material;
  }

  void setTopTraceMaterial(Material const& new_value)
  {
    this->top_trace_material = new_value;
  }

  Material const& getTopPadMaterial() const
  {
    return top_pad_material;
  }

  void setTopPadMaterial(Material const& new_value)
  {
    this->top_pad_material = new_value;
  }
  
  double getTopCopperThickness() const
  {
    return top_copper_thickness;
  }

  void setTopCopperThickness(double new_value)
  {
    this->top_copper_thickness = new_value;
  }

  Material const& getTopMaskMaterial() const
  {
    return top_mask_material;
  }

  void setTopMaskMaterial(Material const& new_value)
  {
    this->top_mask_material = new_value;
  }

  double getTopMaskThickness() const
  {
    return top_mask_thickness;
  }

  void setTopMaskThickness(double new_value)
  {
    this->top_mask_thickness = new_value;
  }

  Material const& getTopSilkMaterial() const
  {
    return top_silk_material;
  }

  void setTopSilkMaterial(Material const& new_value)
  {
    this->top_silk_material = new_value;
  }

  double getTopSilkThickness() const
  {
    return top_silk_thickness;
  }

  void setTopSilkThickness(double new_value)
  {
    this->top_silk_thickness = new_value;
  }

  Material const& getBottomTraceMaterial() const
  {
    return bottom_trace_material;
  }

  void setBottomTraceMaterial(Material const& new_value)
  {
    this->bottom_trace_material = new_value;
  }

  Material const& getBottomPadMaterial() const
  {
    return bottom_pad_material;
  }

  void setBottomPadMaterial(Material const& new_value)
  {
    this->bottom_pad_material = new_value;
  }

  double getBottomCopperThickness() const
  {
    return bottom_copper_thickness;
  }

  void setBottomCopperThickness(double new_value)
  {
    this->bottom_copper_thickness = new_value;
  }

  Material const& getBottomMaskMaterial() const
  {
    return bottom_mask_material;
  }

  void setBottomMaskMaterial(Material const& new_value)
  {
    this->bottom_mask_material = new_value;
  }

  double getBottomMaskThickness() const
  {
    return bottom_mask_thickness;
  }

  void setBottomMaskThickness(double new_value)
  {
    this->bottom_mask_thickness = new_value;
  }

  Material const& getBottomSilkMaterial() const
  {
    return bottom_silk_material;
  }

  void setBottomSilkMaterial(Material const& new_value)
  {
    this->bottom_silk_material = new_value;
  }

  double getBottomSilkThickness() const
  {
    return bottom_silk_thickness;
  }

  void setBottomSilkThickness(double new_value)
  {
    this->bottom_silk_thickness = new_value;
  }

  static Process const& DefaultProcess()
  {
    Process process = Process();
    default_process = process;
    return default_process;
  }
private:
  //
  // N.B.: all thicknesses in inches
  //
  std::string name;

  Material substrate_material;
  double substrate_thickness;

  Material hole_material;

  Material top_trace_material;
  Material top_pad_material;
  double top_copper_thickness;

  Material bottom_trace_material;
  Material bottom_pad_material;
  double bottom_copper_thickness;

  Material top_mask_material;
  double top_mask_thickness;

  Material bottom_mask_material;
  double bottom_mask_thickness;

  Material top_silk_material;
  double top_silk_thickness;

  Material bottom_silk_material;  
  double bottom_silk_thickness;

  Material mill_material;  

  static Process default_process;
};

#endif // #ifndef PROCESS_HPP_INCLUDED_
