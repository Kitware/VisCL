#include "hessian.h"

#include <boost/make_shared.hpp>

#include "cl_task_registry.h"
#include "cl_manager.h"

#include "gaussian_smooth.h"

extern const char* hessian_source;

//*****************************************************************************

hessian::hessian() : cl_task(hessian_source)
{
  det_hessian = make_kernel("det_hessian");
  detect_extrema = make_kernel("detect_extrema");
}

//*****************************************************************************

hessian *hessian::clone()
{
  hessian *clone_ = new hessian(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

template <class T>
void hessian::detect(const vil_image_view<T> &img, unsigned int max_keypoints) const
{
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_image kptmap = detect(img_cl, max_keypoints);

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(img.ni());
  region.push_back(img.nj());
  region.push_back(1);

  vil_image_view<vxl_byte> output(img.ni(), img.nj());
  queue->enqueueReadImage(*kptmap().get(),  CL_TRUE, origin, region, 0, 0, (float *)output.top_left_ptr());

  unsigned int count = 0;
  vcl_ofstream outfile("kpts.txt");
  for (unsigned int i = 0; i < output.ni(); i++)
  {
    for (unsigned int j = 0; j < output.nj(); j++)
    {
      vcl_cout << (int)output(i,j) << " ";
      if (output(i,j)) {
        outfile << i << " " << j << "\n";
        count++;
      }
    }
  }

  vcl_cout << count << "\n";
  outfile.close();
}

//*****************************************************************************

cl_image hessian::detect(const cl_image &img, unsigned int max_keypoints) const
{
  gaussian_smooth_t gs = NEW_TASK(gaussian_smooth);
  cl_image smoothed = gs->smooth(img, 2.0);

  size_t ni = img.ni(), nj = img.nj();
  cl::ImageFormat detimg_fmt(CL_INTENSITY, CL_FLOAT);
  cl_image detimg = cl_manager::inst()->create_image(detimg_fmt, CL_MEM_READ_WRITE, ni, nj);

  cl::ImageFormat kptmap_fmt(CL_R, CL_UNSIGNED_INT8);
  cl_image kptmap = cl_manager::inst()->create_image(kptmap_fmt, CL_MEM_WRITE_ONLY, ni, nj);

  // Set arguments to kernel
  det_hessian->setArg(0, *smoothed().get());
  det_hessian->setArg(1, *detimg().get());
  
  // Set arguments to kernel
  detect_extrema->setArg(0, *detimg().get());
  detect_extrema->setArg(1, *kptmap().get());
  
  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  cl::NDRange local(1,1);

  queue->enqueueNDRangeKernel(*det_hessian.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueBarrier();
  queue->enqueueNDRangeKernel(*detect_extrema.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();

  return kptmap;
}

//*****************************************************************************

template void hessian::detect(const vil_image_view<float> &img, unsigned int max_keypoints) const;
template void hessian::detect(const vil_image_view<vxl_byte> &img, unsigned int max_keypoints) const;
