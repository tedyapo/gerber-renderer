#ifndef IMAGE_OPS_H_
#define IMAGE_OPS_H_

template<typename T>
T min(const T &a, const T&b){
  return a<b ? a : b;
}

template<typename T>
T max(const T &a, const T&b){
  return a>b ? a : b;
}

/* moved to Vector3.h
template <typename pixel_t>
Vector3<pixel_t> min(const Vector3<pixel_t> &x, const Vector3<pixel_t> &y){
  return Vector3<pixel_t>(min(x.r(), y.r()),
			  min(x.g(), y.g()),
			  min(x.b(), y.b()));
}

template <typename pixel_t>
Vector3<pixel_t> max(const Vector3<pixel_t> &x, const Vector3<pixel_t> &y){
  return Vector3<pixel_t>(max(x.r(), y.r()),
			  max(x.g(), y.g()),
			  max(x.b(), y.b()));
}
*/

//!!! note: could also keep track of coordinates of min, max
template <typename Pixel>
struct ImageExtrema {
  typedef Pixel Pixel_t;
  ImageExtrema(){
    min_value = PixelTraits<Pixel_t>::maximum_value();
    max_value = PixelTraits<Pixel_t>::minimum_value();
  }
  void operator()(const Pixel_t &pixel, int row, int col){
    min_value = min(pixel, min_value);
    max_value = max(pixel, max_value);
  }
  Pixel_t getMin() { return min_value; }
  Pixel_t getMax() { return max_value; }
private:
  Pixel_t min_value;
  Pixel_t max_value;
};

#define ImageMin ImageExtrema
#define ImageMax ImageExtrema

template <typename Pixel>
struct ImageClearer {
  typedef Pixel Pixel_t;
  ImageClearer(Pixel_t val){
    this->val = val;
  }
  void operator()(Pixel_t &pixel, int row, int col){
    pixel = val;
  }
private:
  Pixel_t val;
};


template <typename label_t>
class CCLookup {
  CCLookup(int block_size){
    this->block_size = block_size;
    max_label = block_size;
    table = new label_t[max_label];
    for (int i=0; i<max_label; ++i){
      table[i] = label_t(i);
    }
    current_label = label_t(0);
  }

  CCLookup& operator=(const CCLookup &l){
    FATAL_ERROR("unimplemented");
  }

  CCLookup(const CCLookup &l){
    FATAL_ERROR("unimplemented");
  }

  ~CCLookup(){
    delete [] table;
  }

  label_t next_label(){
    current_label++;
    if (current_label > max_label-1){
      int old_max_label = max_label;
      while (current_label > max_label-1){
	max_label += block_size;      
      }
      label_t *new_table = new label_t[max_label];
      int i;
      for (i=0; i<old_max_label; ++i){
	new_table[i] = table[i];
      }
      for (;i<max_label; ++i){
	new_table[i] = label_t(i);
      }
      delete [] table;
      table = new_table;
    }
    return current_label;
  }

  label_t& operator()(int index){
    return table[index];
  }

  // find minimum label, ignoring zeros
  label_t nzmin(label_t a, label_t b){
    if (label_t(0) == a){
      return b;
    } else if (label_t(0) == b){
      return a;
    }
    return a<b ? a : b;
  }

  // merge two labels
  void merge(label_t a, label_t b){
    if (label_t(0) != a && label_t(0) != b){
      label_t c = nzmin(find(a), find(b));
      table[a] = c;
      table[b] = c;
    }
  }

  // find minimum equivalent label
  label_t find(label_t a){
    if (table[a] == a){
      return a;
    } else {
      table[a] = find(table[a]);
      return table[a];
    }
  }
  
  // "reduce" labels to minimum equivalent
  void reduce(){
    int num_labels = 0;
    for (int i=0;i<max_label;++i){
      if (table[i] == i){
	table[i] = num_labels++;
      } else {
	table[i] = table[int(table[i])];
      }
    }
  }

  label_t current_label;
  int max_label;
  int block_size;
  label_t *table;

  template <typename p_t, typename l_t>
  friend Image<l_t> ConnectedComponents(const Image<p_t>, l_t&);
};

template <typename pixel_t, typename label_t>
Image<label_t> ConnectedComponents(const Image<pixel_t> image,
				   label_t &num_labels){

  Image<label_t> labels(image.getRows(), image.getCols());
  ImageClearer<label_t> clearer(label_t(0));
  ImagePointOperation(labels, clearer);
  
  // simple heuristic for lookup table block allocation
  int block_size = image.getRows() + image.getCols();
  CCLookup<label_t> lookup(block_size);

  if (image.getRows() < 1 || image.getCols() < 2){
    FATAL_ERROR("Image too small for connected components");
  }

  //
  // intial labeling pass through image
  //

  // first pixel
  if (pixel_t(0) != image(0,0)){
    labels(0,0) = lookup.next_label();
  }
  // first row
  for (int col=1; col<image.getCols(); ++col){
    if (pixel_t(0) != image(0, col)){
      if (label_t(0) != labels(0, col-1)){
	labels(0, col) = labels(0, col-1);
      } else {
	labels(0, col) = lookup.next_label();
      }
    }
  }
  // rest of rows
  for (int row=1; row<image.getRows(); ++row){
    // first col
    if (pixel_t(0) != image(row, 0)){
      label_t min_label = labels(row-1, 0);
      min_label = lookup.nzmin(min_label, labels(row-1, 1));      
      if (label_t(0) == min_label){
	labels(row, 0) = lookup.next_label();
      } else {
	min_label = lookup.find(min_label);
	labels(row, 0) = min_label;
	lookup.merge(labels(row-1, 0), min_label);
	lookup.merge(labels(row-1, 1), min_label);
      }
    }
    // middle cols
    int col;
    for (col=1; col<image.getCols()-1; ++col){
      if (pixel_t(0) != image(row, col)){
	label_t min_label = labels(row, col-1);
	min_label = lookup.nzmin(min_label, labels(row-1, col-1));
	min_label = lookup.nzmin(min_label, labels(row-1, col));      
	min_label = lookup.nzmin(min_label, labels(row-1, col+1));      
	if (label_t(0) == min_label){
	  labels(row, col) = lookup.next_label();
	} else {
	  min_label = lookup.find(min_label);
	  labels(row, col) = min_label;
	  lookup.merge(labels(row, col-1), min_label);
	  lookup.merge(labels(row-1, col-1), min_label);
	  lookup.merge(labels(row-1, col), min_label);
	  lookup.merge(labels(row-1, col+1), min_label);
	}
      }
    }
    // last col
    if (pixel_t(0) != image(row, col)){
      label_t min_label = labels(row, col-1);
      min_label = lookup.nzmin(min_label, labels(row-1, col-1));
      min_label = lookup.nzmin(min_label, labels(row-1, col));      
      if (label_t(0) == min_label){
	labels(row, col) = lookup.next_label();
      } else {
	min_label = lookup.find(min_label);
	labels(row, col) = min_label;
	lookup.merge(labels(row, col-1), min_label);
	lookup.merge(labels(row-1, col-1), min_label);
	lookup.merge(labels(row-1, col), min_label);
      }
    }
  }

  //
  // reverse merge pass
  //
  for (int row=labels.getRows()-2; row>0; row--){
    for (int col=labels.getCols()-2; col>0; col--){
      if (pixel_t(0) != image(row,col)){
	label_t min_label = labels(row, col);
	min_label = lookup.nzmin(min_label, labels(row-1, col-1));
	min_label = lookup.nzmin(min_label, labels(row-1, col));      
	min_label = lookup.nzmin(min_label, labels(row-1, col+1));      
	min_label = lookup.nzmin(min_label, labels(row, col-1));
	min_label = lookup.nzmin(min_label, labels(row, col+1));
	min_label = lookup.nzmin(min_label, labels(row+1, col-1));
	min_label = lookup.nzmin(min_label, labels(row+1, col));      
	min_label = lookup.nzmin(min_label, labels(row+1, col+1));    
	min_label = lookup.find(min_label);
	lookup.merge(labels(row-1, col-1), min_label);
	lookup.merge(labels(row-1, col), min_label);
	lookup.merge(labels(row-1, col+1), min_label);  
	lookup.merge(labels(row, col-1), min_label);
	lookup.merge(labels(row, col), min_label);
	lookup.merge(labels(row, col+1), min_label);
	lookup.merge(labels(row+1, col-1), min_label);
	lookup.merge(labels(row+1, col), min_label);
	lookup.merge(labels(row+1, col+1), min_label);
      }
    }
  }

  // remove numbering gaps
  lookup.reduce();

  //
  // pass through image to re-number pixel labels
  //
  num_labels = 0;
  for (int row=0; row<labels.getRows(); ++row){
    for (int col=0; col<labels.getCols(); ++col){
      labels(row,col) = lookup(labels(row,col));
      if (labels(row,col) > num_labels) num_labels = labels(row,col);
    }
  }
  num_labels++;

  return labels;
}

template <typename pixel_t>
Image<pixel_t> sRGBtoGray(const Image<VectorN<pixel_t, 3> > &in){
  Image<pixel_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); ++row){
    for (int col=0; col<in.getCols(); ++col){
      out(row, col) = pixel_t(0.299 * in(row,col)(0) +
			      0.587 * in(row,col)(1) +
			      0.114 * in(row,col)(2));
    }
  }
  return out;
}

template <typename pixel_t>
Image<pixel_t> ThresholdImage(const Image<pixel_t> &in, pixel_t threshold){
  PixelTraits<pixel_t> traits;
  Image<pixel_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      if (in(row,col) > threshold){
	out(row,col) = traits.maximum_value();
      } else {
	out(row,col) = traits.minimum_value();
      }
    }
  }
  return out;
}

// write a histogram stretcher which gets constructed from either min, max values
// or directly from an ImageExtrema class to stretch fully.

//!!! how to write a generic template-based efficient neighborhood-op function ???
#endif // #ifndef IMAGE_OPS_H_
