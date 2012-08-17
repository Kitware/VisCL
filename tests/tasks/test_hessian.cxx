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

#include <viscl/tasks/hessian.h>

#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
#include <vil/vil_crop.h>
#include <vil/vil_convert.h>
#include <vil/algo/vil_find_peaks.h>

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

static void test_det_image();
static void test_peaks();

void
run_test(std::string const& test_name)
{
  if (test_name == "det_image")
  {
    test_det_image();
  }
  else if (test_name == "peaks")
  {
    test_peaks();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}


/// Make a non-uniform checkerboard-like pattern image for use in tests.
/// Fills a pre-allocated buffer \a buffer of size \a width by \a height.
void make_wave_image(const unsigned width,
                     const unsigned height,
                     unsigned char* const buffer,
                     const double freq = 8.0)
{
  for (unsigned j=0; j<height; ++j)
  {
    for (unsigned i=0; i<width; ++i)
    {
      // index into the image buffer
      const unsigned index = j * width + i;
      // compute the wave pattern
      double val = (sin(i*i*freq / width) * cos(j*j*freq / height) + 1) / 2.0;
      // scale bool to byte image range
      buffer[index] = static_cast<unsigned char>(val * 255);
    }
  }
}


/// Make a Hessian determinant image of the above wave function
/// Fills a pre-allocated buffer \a buffer of size \a width by \a height.
void make_hesdet_wave_image(const unsigned width,
                            const unsigned height,
                            float* const buffer,
                            const double freq = 8.0)
{
  for (unsigned j=0; j<height; ++j)
  {
    for (unsigned i=0; i<width; ++i)
    {
      // index into the image buffer
      const unsigned index = j * width + i;
      // compute the wave pattern
      double f1 = freq / width;
      double f2 = freq / height;
      double iif1 = i*i*f1;
      double jjf2 = j*j*f2;
      double c1 = cos(iif1);
      double c2 = cos(jjf2);
      double s1 = sin(iif1);
      double s2 = sin(jjf2);
      double val = 4*iif1*jjf2*(c2*c2 - c1*c1);
      val += 2*iif1*(s2*c2 -s2*c2*c1*c1);
      val -= 2*jjf2*(s1*c1*c2*c2) + s1*s2*c1*c2;
      val *= f1*f2;
      // scale bool to byte image range
      buffer[index] = static_cast<float>(val);
    }
  }
}


// test the determinant of Hessian image
void
test_det_image()
{
  // test parameters
  const unsigned width = 100;
  const unsigned height = 100;
  const float freq = 0.3f;

  // create the viscl task
  viscl::hessian_t hes = NEW_VISCL_TASK(viscl::hessian);

  // create a test image with a sinusoidal pattern
  const size_t buffer_size = width * height;
  cl::ImageFormat img_frmt = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);
  unsigned char img_data[buffer_size];
  make_wave_image(width, height, img_data, freq);

  // compute the analytical Hessian determinant solution
  float hes_data[buffer_size];
  make_hesdet_wave_image(width, height, hes_data, freq);

  // create the image on the GPU and upload the test image to it.
  viscl::image img = viscl::manager::inst()->create_image(img_frmt, CL_MEM_READ_ONLY,
                                                          width, height);
  viscl::cl_queue_t queue = hes->get_queue();

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

  // compute the Hessian determinant on the GPU
  viscl::image detimg;
  hes->det_hessian_image(img, detimg, 1.0);

  // read the image back to the CPU
  float detimg_data[buffer_size];
  queue->enqueueReadImage(*detimg().get(), CL_TRUE, origin, region,
                           0, 0, detimg_data);

  // use vil to wrap the images for comparisons
  vil_image_view<vxl_byte> wave(img_data, width, height, 1, 1, width, width*height);
  vil_image_view<float> hes_wave(hes_data, width, height, 1, 1, width, width*height);
  vil_image_view<float> hes_wave2(detimg_data, width, height, 1, 1, width, width*height);
  vil_image_view<float> diff;
  vil_math_image_abs_difference(hes_wave, hes_wave2, diff);
  // ignore the boundary pixels
  diff = vil_crop(diff, 1, width-2, 1, height-2);

  // find the range of values in both images
  float min_v, max_v, min_t, max_t;
  vil_math_value_range(hes_wave, min_v, max_v);
  std::cout << "hes_wave ranges from "<<min_v<<" to "<<max_v<<std::endl;
  min_t = min_v;
  max_t = max_v;
  vil_math_value_range(hes_wave2, min_v, max_v);
  std::cout << "hes_wave2 ranges from "<<min_v<<" to "<<max_v<<std::endl;
  min_t = std::min(min_t, min_v);
  max_t = std::max(max_t, max_v);

  // find 3 standard deviations above the mean error
  double mean, var;
  vil_math_mean_and_variance(mean, var, diff, 0);
  double norm_error = (mean + 3*sqrt(var)) / (max_t - min_t);
  // ideally this threshold should be lower ...
  if (norm_error > 0.05)
  {
    TEST_ERROR("Determinant of Hessian is not accurate.  "
               "Normalized error is " << norm_error);

    // write debug images
    vil_image_view<vxl_byte> byte_img;
    vil_convert_stretch_range_limited(hes_wave, byte_img, min_t, max_t);
    vil_save(byte_img, "hes_wave.png");
    vil_convert_stretch_range_limited(hes_wave2, byte_img, min_t, max_t);
    vil_save(byte_img, "hes_wave2.png");

    vil_convert_stretch_range(wave, byte_img);
    vil_save(byte_img, "wave.png");
  }
}


// compare the peaks to vil's peak detection
void
test_peaks()
{
  // test parameters
  const unsigned width = 100;
  const unsigned height = 100;
  const float freq = 0.3f;
  const float thresh = 0.003f;

  // create the viscl task
  viscl::hessian_t hes = NEW_VISCL_TASK(viscl::hessian);

  // create a test image with a sinusoidal pattern
  const size_t buffer_size = width * height;
  cl::ImageFormat img_frmt = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);
  unsigned char img_data[buffer_size];
  make_wave_image(width, height, img_data, freq);

  // create the image on the GPU and upload the test image to it.
  viscl::image img = viscl::manager::inst()->create_image(img_frmt, CL_MEM_READ_ONLY,
                                                          width, height);
  viscl::cl_queue_t queue = hes->get_queue();

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

  // compute the Hessian determinant image from the test image
  viscl::image detimg;
  hes->det_hessian_image(img, detimg, 1.0);

  // download the Hessian determinant image
  float detimg_data[buffer_size];
  queue->enqueueReadImage(*detimg().get(), CL_TRUE, origin, region,
                           0, 0, detimg_data);

  // detect points
  viscl::image kptmap;
  viscl::buffer numkpts_b, kpts_b, kvals_b;
  hes->detect(img, kptmap, kpts_b, kvals_b, numkpts_b,
              thresh, 1.0, false);

  int buf[1];
  region[0] = width/2;
  region[1] = height/2;

  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  int numkpts = buf[0];

  std::vector<cl_float2> kpts(numkpts);
  std::vector<float> kvals(numkpts);
  queue->enqueueReadBuffer(*kpts_b().get(), CL_TRUE, 0, sizeof(cl_float2)*numkpts, &kpts[0]);
  queue->enqueueReadBuffer(*kvals_b().get(), CL_TRUE, 0, sizeof(float)*numkpts, &kvals[0]);
  int kptmap_img[buffer_size / 4];
  queue->enqueueReadImage(*kptmap().get(), CL_TRUE, origin, region,
                           0, 0, kptmap_img);

  // detect subpixel points
  hes->detect(img, kptmap, kpts_b, kvals_b, numkpts_b,
              thresh, 1.0, true);

  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  int numkpts_sub = buf[0];

  std::vector<cl_float2> kpts_sub(numkpts);
  std::vector<float> kvals_sub(numkpts);
  queue->enqueueReadBuffer(*kpts_b().get(), CL_TRUE, 0, sizeof(cl_float2)*numkpts, &kpts_sub[0]);
  queue->enqueueReadBuffer(*kvals_b().get(), CL_TRUE, 0, sizeof(float)*numkpts, &kvals_sub[0]);
  int kptmap_img_sub[buffer_size / 4];
  queue->enqueueReadImage(*kptmap().get(), CL_TRUE, origin, region,
                           0, 0, kptmap_img_sub);


  // use vil to wrap the determinant image and compute peaks
  vil_image_view<float> det_hes(detimg_data, width, height, 1, 1, width, width*height);
  std::vector<unsigned> pi,pj;
  vil_find_peaks_3x3(pi, pj, det_hes, thresh);
  std::vector<double> spi,spj,sval;
  vil_find_peaks_3x3_subpixel(spi, spj, sval, det_hes, thresh);

  // There can never be more sub-pixel peaks than pixel peaks
  if (numkpts < numkpts_sub)
  {
    TEST_ERROR("More sub-pixel detections than pixel detections: "
               << numkpts_sub << " and " << numkpts << " resp.");
  }

  // compare numbers of vil peaks to viscl peaks
  if (numkpts != pi.size())
  {
    TEST_ERROR("Different number of viscl and VXL peaks detected: "
               << numkpts << " and " << pi.size() << " resp.");
  }

  if (numkpts_sub != spi.size())
  {
    TEST_ERROR("Different number of viscl and VXL sub-pixel peaks detected: "
               << numkpts_sub << " and " << spi.size() << " resp.");
  }

  // make sure pixel peaks are the same between vil and viscl
  bool found_same = true;
  for (unsigned int k=0; k<pi.size(); ++k)
  {
    const unsigned index = pj[k]/2 * width/2 + pi[k]/2;
    if (kptmap_img[index] < 0)
    {
      found_same = false;
      std::cout << "no VXL peak at "<<pi[k]<<", "<<pj[k]<<std::endl;
      break;
    }
  }
  if (!found_same)
  {
    TEST_ERROR("VXL did not find same peaks as viscl");
  }

  // make sure sub-pixel peaks are the same between vil and viscl
  found_same = true;
  bool same_vals = true;
  for (unsigned int k=0; k<spi.size(); ++k)
  {
    // interpolation can move a pixel up to 1 pixel away from its initial location
    // explore the keypoint map in a local neighborhood to find the matching viscl point
    const unsigned i1 = std::max(static_cast<int>(spi[k]-1.0) / 2, 0);
    const unsigned i2 = std::min(static_cast<int>(spi[k]+1.0) / 2, static_cast<int>(width/2-1));
    const unsigned j1 = std::max(static_cast<int>(spj[k]-1.0) / 2, 0);
    const unsigned j2 = std::min(static_cast<int>(spj[k]+1.0) / 2, static_cast<int>(height/2-1));
    bool match = false;
    for (unsigned int j=j1; !match && j<=j2; ++j)
    {
      for (unsigned int i=i1; !match && i<=i2; ++i)
      {
        const unsigned index = j * width/2 + i;
        const int idx = kptmap_img_sub[index];
        if (idx < 0)
        {
          continue;
        }
        const float dx = spi[k] - kpts_sub[idx].s0;
        const float dy = spj[k] - kpts_sub[idx].s1;
        if (sqrt(dx*dx + dy*dy) < 1e-4)
        {
          match = true;
          // check that keypoints also have the same magnitudes
          if (std::fabs(kvals_sub[idx] - sval[k]) > 1e-4)
          {
            same_vals = false;
            std::cerr << "viscl and VXL peak magnitudes differ: "
                      << kvals_sub[idx] <<" and " <<sval[k] << " resp."
                      << std::endl;
          }
        }
      }
    }
    if (!match)
    {
      found_same = false;
      std::cerr << "no VXL sub-pixel peak at "<<spi[k]<<", "<<spj[k]<<std::endl;
      break;
    }
  }
  if (!found_same)
  {
    TEST_ERROR("VXL did not find same sub-pixel peaks as viscl");
  }


}
