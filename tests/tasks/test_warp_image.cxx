/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <iostream>
#include <cmath>
#include <cstdlib>
#include <exception>

#include <test_common.h>

#include <viscl/core/manager.h>

#include <viscl/tasks/warp_image.h>

#ifdef HAS_VXL
#include <viscl/vxl/transfer.h>
#include <viscl/vxl/tasks.h>
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vil/vil_save.h>
#include <vil/vil_bilin_interp.h>
#include <vil/vil_nearest_interp.h>

bool
warp_image( vil_image_view<vxl_byte> const& src,
            vil_image_view<vxl_byte>& dest,
            vgl_h_matrix_2d<float> const& dest_to_src_homography);

#endif
static void run_test(std::string const& test_name);

int
main(int argc, char* argv[])
{
  if (argc != 2)
  {
    TEST_ERROR("Expected one argument");

    return EXIT_FAILURE;
  }

  std::string const test_name = argv[1];

  try
  {
    run_test(test_name);
  }
  catch (std::exception const& e)
  {
    TEST_ERROR("Unexpected exception: " << e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static void test_warp_vxl();

void
run_test(std::string const& test_name)
{
  if (test_name == "warp_vxl")
  {
    test_warp_vxl();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}

void
test_warp_vxl()
{
#ifdef HAS_VXL
  // test parameters
  const unsigned width = 1000;
  const unsigned height = 1000;

  // create a test image with a checkerboard pattern
  vil_image_view<vxl_byte> input_img(width, height), result_img;
  make_checkerboard_image(width, height, input_img.top_left_ptr(), 4);

  vgl_h_matrix_2d<float> H;
  H.set_identity();
  H.set_rotation(1.0f);
  H.set_translation(0.0f, -10.0f);

  vcl_cout << H << "\n";

  // warp the image with the CPU
  vil_image_view<vxl_byte> truth_img(width, height);
  warp_image(input_img, truth_img, H);

  // apply the viscl warping task
  try
  {
    viscl::warp_image_vxl(input_img, result_img, H);
  }
  catch (const cl::Error &e)
  {
    TEST_ERROR(e.what() << " (" << e.err() << " : "
               << viscl::print_cl_errstring(e.err()) << ")");
  }

  unsigned long diff_count = 0;
  for( unsigned j = 0; j < height; ++j )
  {
    for( unsigned i = 0; i < width; ++i )
    {
      if (result_img(i,j) != truth_img(i,j))
      {
        ++diff_count;
      }
    }
  }

  if (diff_count > 0)
  {
    TEST_ERROR("GPU and CPU warping results differ in "
               << (100.0f*diff_count)/(width*height) << "% of pixels");

    //vil_save(input_img, "test_warp_source.png");
    vil_save(result_img, "test_warp_result.png");
    vil_save(truth_img, "test_warp_truth.png");
  }

#else
  TEST_ERROR("VXL support is not compiled in, this test is invalid");
#endif
}

bool
warp_image( vil_image_view<vxl_byte> const& src,
            vil_image_view<vxl_byte>& dest,
            vgl_h_matrix_2d<float> const& dest_to_src_homography)
{
  // Retrieve destination and source image properties
  unsigned const dni = dest.ni();
  unsigned const dnj = dest.nj();
  unsigned const dnp = dest.nplanes();
  unsigned const sni = src.ni();
  unsigned const snj = src.nj();
  unsigned const snp = src.nplanes();

  // Source and destination must have the same number of channels
  assert( snp == dnp );

  // Cast unmapped parameter value
  vxl_byte const unmapped_value = static_cast<vxl_byte>( 0 );

  typedef vgl_homg_point_2d<float> homog_type;
  typedef vgl_point_2d<float> point_type;
  typedef vgl_box_2d<float> box_type;

  // First, figure out the bounding box of src projected into dest.
  // There may be considerable computation saving for the cases when
  // the output image is much larger that the projected source image,
  // which will often occur during mosaicing.

  vgl_h_matrix_2d<float> const& src_to_dest_homography(
    vnl_inverse( dest_to_src_homography.get_matrix() ) );

  box_type src_on_dest_bounding_box;

  homog_type cnrs[4] = { homog_type( 0, 0 ),
                         homog_type( sni - 1, 0 ),
                         homog_type( sni - 1, snj - 1 ),
                         homog_type( 0, snj - 1 ) };

  for( unsigned i = 0; i < 4; ++i )
  {
    // Shift the point to destination image pixel index coordinates
    point_type p = src_to_dest_homography * cnrs[i];
    src_on_dest_bounding_box.add( p );
  }

  // Calculate intersection with destination pixels we are looking to fill
  box_type dest_boundaries( 0, dni - 1, 0, dnj - 1 );
  box_type intersection = vgl_intersection( src_on_dest_bounding_box, dest_boundaries );

  // Fill in unmapped mask and destination with default values. Maybe
  // implement a couple of loops to fill only the "boundary" regions with
  // default values later.
  dest.fill( unmapped_value );

  int src_ni_low_bound, src_nj_low_bound, src_ni_up_bound, src_nj_up_bound;
  src_ni_low_bound = 0;
  src_nj_low_bound = 0;
  src_ni_up_bound = sni - 1;
  src_nj_up_bound = snj - 1;

  // Extract start and end, row/col scanning ranges [start..end-1]
  const int start_j = static_cast<int>(std::floor(intersection.min_y()));
  const int start_i = static_cast<int>(std::floor(intersection.min_x()));
  const int end_j = static_cast<int>(std::floor(intersection.max_y()+1));
  const int end_i = static_cast<int>(std::floor(intersection.max_x()+1));

  // Create adjusted start and end scanning values for supplied offset
  const int start_j_adj = start_j;
  const int start_i_adj = start_i;
  const int end_j_adj = end_j;
  const int end_i_adj = end_i;

  // Get pointers to image data and retrieve required step values
  vxl_byte* row_start = dest.top_left_ptr();
  const vxl_byte* src_start = src.top_left_ptr();
  const vcl_ptrdiff_t dest_j_step = dest.jstep();
  const vcl_ptrdiff_t src_j_step = src.jstep();
  const vcl_ptrdiff_t dest_i_step = dest.istep();
  const vcl_ptrdiff_t src_i_step = src.istep();
  const vcl_ptrdiff_t dest_p_step = dest.planestep();
  const vcl_ptrdiff_t src_p_step = src.planestep();
  const vxl_byte* src_p_end = src_start + snp * src_p_step;

  // Adjust destination itr position to start of bounding box
  row_start = row_start + start_j * dest_j_step + dest_i_step * start_i;

  // Precompute partial column homography values
  int factor_size = end_i_adj - start_i_adj;

  vnl_matrix_fixed<float,3,3> homog = dest_to_src_homography.get_matrix();
  vnl_vector_fixed<float, 3> homog_col_1 = homog.get_column( 0 );
  vnl_vector_fixed<float, 3> homog_col_2 = homog.get_column( 1 );
  vnl_vector_fixed<float, 3> homog_col_3 = homog.get_column( 2 );

  vcl_vector< vnl_vector_fixed<float, 3> > col_factors( factor_size );

  for( int i = 0; i < factor_size; i++ )
  {
    float col = float( i + start_i_adj );
    col_factors[ i ] = col * homog_col_1 + homog_col_3;
  }

  // Perform scan of boxed region
  for( int j = start_j_adj; j < end_j_adj; j++, row_start += dest_j_step )
  {
    // dest_col_ptr now points to the start of the BB region for this row
    vxl_byte* dest_col_ptr = row_start;

    // Precompute row homography partials for this row
    const vnl_vector_fixed<float, 3> row_factor = homog_col_2 * (float)j;

    // Get pointer to start of precomputed column values
    vnl_vector_fixed<float, 3>* col_factor_ptr = &col_factors[0];

    // Iterate through each column in the BB
    for( int i = start_i_adj; i < end_i_adj; i++, col_factor_ptr++, dest_col_ptr += dest_i_step )
    {

      // Compute homography mapping for this point (dest->src)
      vnl_vector_fixed<float, 3> pt = row_factor + (*col_factor_ptr);

      // Normalize by dividing out third term
      float& x = pt[0];
      float& y = pt[1];
      float& w = pt[2];

      x /= w;
      y /= w;

      // Check if we can perform interp at this point
      if( !(x < src_ni_low_bound || y < src_nj_low_bound || x > src_ni_up_bound || y > src_nj_up_bound) )
      {

        // For each channel interpolate from src
        const vxl_byte* src_plane = src_start;
        vxl_byte* dest_pixel_ptr = dest_col_ptr;
        for( ; src_plane < src_p_end; src_plane += src_p_step, dest_pixel_ptr += dest_p_step)
        {
          *dest_pixel_ptr = vil_bilin_interp_raw( x, y, src_plane, sni, snj, src_i_step, src_j_step );
        }
      }
    }
  }

  return true;
}
