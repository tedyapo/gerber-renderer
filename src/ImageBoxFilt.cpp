#include "Image.h"

template<typename pixel_t>
BoxFilter(Image<pixel_t> &in, Image<pixel_t> &out,
          int filt_half_rows,
          int filt_half_cols)
{
  Image<pixel_t>::accum_pixel_t sum;

  // !!! assert images are same size

  // note: need intermediate image: use smaller image of filt_rows rows
  
  // filter cols first
  Image<pixel_t>::accum_pixel_t
    inv_size(1./((2*filt_half_rows+1)*(2*filt_half_cols+1)));

  Image<Image<pixel_t>::accum_pixel_t>
    col_filtered(2*filt_half_rows+1, in.getCols());

  for (int row=0; row<in.getRows(); ++row)
  {
    int read_row = row%something;
    int write_row = row%something;
    for (int col=0; col<in.getCols(); ++col)
    {
      sum -= in(read_row, col - filt_half_cols);
      sum += in(read_row, col + filt_half_cols);
      col_filtered(write_row, col) = sum;
    }
  }
}
