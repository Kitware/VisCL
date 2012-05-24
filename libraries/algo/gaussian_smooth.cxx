#include "algo/gaussian_smooth.h"

#include <vil/vil_convert.h>
#include <vil/algo/vil_gauss_filter.h>

#include "cl_manager.h"

extern const char* gaussian_smooth_source;

//*****************************************************************************

gaussian_smooth::gaussian_smooth() : cl_task()
{
  compile(gaussian_smooth_source);
  add_kernel("conv_x");
  add_kernel("conv_y");
}

//*****************************************************************************

template<class T>
void gaussian_smooth::smooth(vil_image_view<T> &img, float sigma)
{
  vil_image_view<vxl_byte> bimg;
  vil_convert_cast<T, vxl_byte>(img, bimg);
  smooth(bimg, sigma);
}

//*****************************************************************************

template<>
void gaussian_smooth::smooth(vil_image_view<vxl_byte> &img, float sigma)
{
  cl::Image2D *img_cl = cl_manager::inst()->create_image(img, true);
  smooth(img_cl, sigma);
  delete img_cl;
}

//*****************************************************************************

void gaussian_smooth::smooth(cl::Image2D *img, float sigma)
{
  vil_gauss_filter_5tap_params params(sigma);
  float *filter = new float[5];
  filter[0] = filter[4] = params.filt2();
  filter[1] = filter[3] = params.filt1();
  filter[2] = params.filt0();

  cl::Buffer *smoothing_kernel = cl_manager::inst()->create_buffer(filter, 5);
 // queue.enqueueWriteBuffer(smoothing_kernel, CL_TRUE, 0, 5 * sizeof(float), filter);

  //  cl::Image2D working(context,
  //                  CL_MEM_READ_WRITE,
  //                  img_fmt,
  //                  grey.ni(),
  //                  grey.nj(),
  //                  0,
  //                  0);

  //  cl::Image2D output(context,
  //                      CL_MEM_WRITE_ONLY,
  //                      img_fmt,
  //                      grey.ni(),
  //                      grey.nj(),
  //                      0,
  //                      0);

  //  cl_uint2 image_dims = {grey.ni(), grey.nj()};

  //  // Set arguments to kernel
  //  kernelHoriz.setArg(0, image);
  //  kernelHoriz.setArg(1, smoothing_kernel);
  //  kernelHoriz.setArg(2, working);    
  //  kernelHoriz.setArg(3, image_dims);

  //  // Set arguments to kernel
  //  kernelVert.setArg(0, working);
  //  kernelVert.setArg(1, smoothing_kernel);
  //  kernelVert.setArg(2, output);    
  //  kernelVert.setArg(3, image_dims);

  //  // Run the kernel on specific ND range
  //  cl::NDRange global(grey.ni(), grey.nj());
  //  cl::NDRange local(1,1);

  //  vcl_ofstream outfile("times.txt");
  //  for (unsigned int times = 1; times <= 100; times++)
  //  {
  //    vcl_cout << times << " ";
  //    outfile << times << " ";
  //    start = GetTickCount();

  //    for (unsigned int i = 0; i < times; i++)
  //    {
  //    queue.enqueueNDRangeKernel(kernelHoriz, cl::NullRange, global, cl::NullRange);
  //    queue.enqueueBarrier();
  //    queue.enqueueNDRangeKernel(kernelVert, cl::NullRange, global, cl::NullRange);
  //    }
  //    cl::size_t<3> origin;
  //    origin.push_back(0);
  //    origin.push_back(0);
  //    origin.push_back(0);

  //    cl::size_t<3> region;
  //    region.push_back(grey.ni());
  //    region.push_back(grey.nj());
  //    region.push_back(1);

  //    queue.enqueueReadImage(output,  CL_TRUE, origin, region, 0, 0, (float *)out_img.top_left_ptr());
}

//*****************************************************************************
