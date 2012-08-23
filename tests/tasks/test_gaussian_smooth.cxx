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
#include <viscl/core/task_registry.h>

#include <viscl/tasks/gaussian_smooth.h>

#ifdef HAS_VXL
#include <viscl/vxl/transfer.h>
#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
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

static void test_smooth();
static void test_smooth_vxl();

void
run_test(std::string const& test_name)
{
  if (test_name == "smooth")
  {
    test_smooth();
  }
  else if (test_name == "smooth_vxl")
  {
    test_smooth_vxl();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}


/// Make a simple checkerboard pattern image for use in tests.
/// Fills a pre-allocated buffer \a buffer of size \a width by \a height.
/// Each block of the checkerboard is \a blocksize by \a blocksize.
void make_checkerboard_image(const unsigned width,
                             const unsigned height,
                             unsigned char* const buffer,
                             const unsigned blocksize = 8)
{
  for (unsigned j=0; j<height; ++j)
  {
    for (unsigned i=0; i<width; ++i)
    {
      // index into the image buffer
      const unsigned index = j * width + i;
      // block cell coordinates
      const unsigned bi = i / blocksize;
      const unsigned bj = j / blocksize;
      // checkerboard boolean value (true for white, false for black)
      const bool val = (bi % 2 + bj % 2) % 2;
      // scale bool to byte image range
      buffer[index] = static_cast<unsigned char>(val * 255);
    }
  }
}


/// Clampe a value \a input between \a min_val and \a max_val.
template <typename T>
T clamp(const T& input, const T& min_val, const T& max_val)
{
  return (input < min_val) ? min_val
                           : ( (input > max_val) ? max_val
                                                 : input );
}


/// Smooth an image with a Gaussian filter on the CPU for comparison with GPU.
void smooth_image(const unsigned width,
                  const unsigned height,
                  const unsigned char* const input,
                  unsigned char* const output,
                  const float sigma, const int kernel_radius)
{
  // Compute the Gaussian kernel
  const int kernel_size = 2 * kernel_radius + 1;
  float *filter = new float[kernel_size];
  int i = 0;
  float sum=0.0f;
  for (int i = 0; i < kernel_size; ++i)
  {
    const float x = static_cast<float>(i - kernel_radius);
    filter[i] = exp( (- x * x) / (2.0f * sigma * sigma));
    sum += filter[i];
  }
  for (int i = 0; i < kernel_size; ++i)
  {
    filter[i] /= sum;
  }

  const size_t buffer_size = width * height;
  const int max_i = width-1;
  const int max_j = height-1;
  unsigned char* work = new unsigned char[buffer_size];
  // smooth horizontally
  // store intermediate results into the working buffer
  for (int j=0; j<height; ++j)
  {
    for (int i=0; i<width; ++i)
    {
      float val = 0.0f;
      for (int offset = -kernel_radius; offset <= kernel_radius; ++offset)
      {
        const int x = clamp(i + offset, 0, max_i);
        val += input[j*width + x] * filter[offset + kernel_radius];
      }
      work[j*width + i] = static_cast<unsigned int>(val+0.5);
    }
  }
  // smooth vertically
  // apply to the working buffer and store in the output buffer
  for (int j=0; j<height; ++j)
  {
    for (int i=0; i<width; ++i)
    {
      float val = 0.0f;
      for (int offset = -kernel_radius; offset <= kernel_radius; ++offset)
      {
        int y = clamp(j + offset, 0, max_j);
        val += work[y*width + i] * filter[offset + kernel_radius];
      }
      output[j*width + i] = static_cast<unsigned int>(val+0.5);
    }
  }
  delete [] filter;
  delete [] work;
}

void
test_smooth()
{
  // test parameters
  const unsigned width = 100;
  const unsigned height = 100;
  const float sigma = 1.0f;
  const int kr = 2;

  // create the viscl task
  viscl::gaussian_smooth_t smoother = NEW_VISCL_TASK(viscl::gaussian_smooth);

  // create a test image with a checkerboard pattern
  const size_t buffer_size = width * height;
  cl::ImageFormat img_frmt = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);
  unsigned char img_data[buffer_size];
  make_checkerboard_image(width, height, img_data);

  // smooth the image with the CPU function for ground truth
  unsigned char truth[buffer_size];
  smooth_image(width, height, img_data, truth, sigma, kr);

  // create the image on the GPU and upload the test image to it.
  viscl::image img = viscl::manager::inst()->create_image(img_frmt, CL_MEM_READ_ONLY,
                                                          width, height);
  viscl::cl_queue_t queue = viscl::manager::inst()->create_queue();

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  queue->enqueueWriteImage(*img().get(), CL_TRUE, origin, region,
                           0, 0, img_data);

  // apply the viscl smoothing task
  viscl::image simg = smoother->smooth(img, sigma, kr);

  // create a result image and download the result to it
  unsigned char result_data[buffer_size];
  queue->enqueueReadImage(*simg().get(), CL_TRUE, origin, region,
                          0, 0, result_data);

  unsigned long diff_count = 0;
  for( unsigned i=0; i<buffer_size; ++i )
  {
    if (result_data[i] != truth[i])
    {
      ++diff_count;
    }
  }
  if (diff_count > 0)
  {
    TEST_ERROR("GPU and CPU smoothing results differ in "
               << (100.0f*diff_count)/buffer_size << "% of pixels");
#ifdef HAS_VXL
    vil_image_view<vxl_byte> vil_src(img_data, width, height, 1,
                                     1, width, buffer_size);
    vil_save(vil_src, "test_smooth_source.png");
    vil_image_view<vxl_byte> vil_result(result_data, width, height, 1,
                                        1, width, buffer_size);
    vil_save(vil_result, "test_smooth_result.png");
    vil_image_view<vxl_byte> vil_truth(truth, width, height, 1,
                                       1, width, buffer_size);
    vil_save(vil_truth, "test_smooth_truth.png");
#endif
  }
}


void
test_smooth_vxl()
{
#ifdef HAS_VXL
  // test parameters
  const unsigned width = 100;
  const unsigned height = 100;
  const float sigma = 1.0f;
  const int kr = 2;

  // create the viscl task
  viscl::gaussian_smooth_t smoother = NEW_VISCL_TASK(viscl::gaussian_smooth);

  // create a test image with a checkerboard pattern
  vil_image_view<vxl_byte> input_img(width, height);
  make_checkerboard_image(width, height, input_img.top_left_ptr());

  // smooth the image with the CPU function for ground truth
  vil_image_view<vxl_byte> truth_img(width, height);
  smooth_image(width, height, input_img.top_left_ptr(),
               truth_img.top_left_ptr(), sigma, kr);

  // create the image on the GPU and upload the test image to it.
  viscl::image img = viscl::upload_image(input_img);

  // apply the viscl smoothing task
  viscl::image simg = smoother->smooth(img, sigma, kr);

  // create a result image and download the result to it
  vil_image_view<vxl_byte> result_img(width, height);
  viscl::cl_queue_t queue = viscl::manager::inst()->create_queue();
  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);
  queue->enqueueReadImage(*simg().get(), CL_TRUE, origin, region,
                          0, 0, result_img.top_left_ptr());

  unsigned long diff_count = 0;
  for( unsigned j=0; j<height; ++j )
  {
    for( unsigned i=0; i<width; ++i )
    {
      if (result_img(i,j) != truth_img(i,j))
      {
        ++diff_count;
      }
    }
  }
  if (diff_count > 0)
  {
    TEST_ERROR("GPU and CPU smoothing results differ in "
               << (100.0f*diff_count)/(width*height) << "% of pixels");

    vil_save(input_img, "test_smooth_source.png");
    vil_save(result_img, "test_smooth_result.png");
    vil_save(truth_img, "test_smooth_truth.png");
  }
#else
  TEST_ERROR("VXL support is not compiled in, this test is invalid");
#endif
}



