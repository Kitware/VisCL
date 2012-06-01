#include "hessian.h"

#include <boost/make_shared.hpp>

#include "cl_task_registry.h"
#include "cl_manager.h"
#include "cl_util.h"

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
  cl_buffer kpts, numkpts;
  detect(img_cl, max_keypoints, kpts, numkpts);

  int num[1];
  queue->enqueueReadBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), num);

  vcl_cout << numkpts.mem_size() << " " << num[0] << " " << kpts.mem_size() << "\n";
  vcl_vector<cl_int2> keypoints(num[0]);
  queue->enqueueReadBuffer(*kpts().get(), CL_TRUE, 0, num[0] * sizeof(cl_int2), keypoints.data());

  vcl_ofstream outfile("kpts.txt");
  for (unsigned int i = 0; i < keypoints.size(); i++)
  {
    outfile << keypoints[i].s[0] << " " << keypoints[i].s[1] << "\n";
  }

  vcl_cout << keypoints.size() << "\n";
  outfile.close();
}

//*****************************************************************************

void hessian::detect(const cl_image &img, unsigned int max_keypoints, cl_buffer &kpts, cl_buffer &numkpts) const
{
  float thresh = 0.0f, scale = 4.0f;
  gaussian_smooth_t gs = NEW_TASK(gaussian_smooth);
  cl_image smoothed = gs->smooth(img, scale);

  int count[1];
  count[0] = 0;
  numkpts = cl_manager::inst()->create_buffer<int>(CL_MEM_READ_WRITE, 1);
  queue->enqueueWriteBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), count);

  kpts = cl_manager::inst()->create_buffer<cl_int2>(CL_MEM_WRITE_ONLY, max_keypoints);

  size_t ni = img.ni(), nj = img.nj();
  cl::ImageFormat detimg_fmt(CL_INTENSITY, CL_FLOAT);
  cl_image detimg = cl_manager::inst()->create_image(detimg_fmt, CL_MEM_READ_WRITE, ni, nj);

  // Set arguments to kernel
  det_hessian->setArg(0, *smoothed().get());
  det_hessian->setArg(1, *detimg().get());
  det_hessian->setArg(2, scale);
  
  // Set arguments to kernel
  detect_extrema->setArg(0, *detimg().get());
  detect_extrema->setArg(1, max_keypoints);
  detect_extrema->setArg(2, thresh);
  detect_extrema->setArg(3, *numkpts().get());
  detect_extrema->setArg(4, *kpts().get());
  
  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  cl::NDRange local(1,1);

  queue->enqueueNDRangeKernel(*det_hessian.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueBarrier();
  queue->enqueueNDRangeKernel(*detect_extrema.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();

  //save_cl_image<float>(queue, detimg, "dethes.png");
}

//*****************************************************************************

template void hessian::detect(const vil_image_view<float> &img, unsigned int max_keypoints) const;
template void hessian::detect(const vil_image_view<vxl_byte> &img, unsigned int max_keypoints) const;
