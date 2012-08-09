/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "hessian.h"

#include <boost/make_shared.hpp>

#include <viscl/core/task_registry.h>
#include <viscl/core/manager.h>

#include "gaussian_smooth.h"


extern const char* hessian_source;

namespace viscl
{

//*****************************************************************************

void hessian::init()
{
  task::build_source(hessian_source);
  det_hessian = make_kernel("det_hessian");
  detect_extrema = make_kernel("detect_extrema");
  init_kpt_map = make_kernel("init_kpt_map");
}

//*****************************************************************************

void hessian::init(const cl_program_t &prog)
{
  program = prog;
  det_hessian = make_kernel("det_hessian");
  detect_extrema = make_kernel("detect_extrema");
  init_kpt_map = make_kernel("init_kpt_map");
}

//*****************************************************************************

task_t hessian::clone()
{
  hessian_t clone_ = boost::make_shared<hessian>(*this);
  clone_->queue = manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

void hessian::smooth_and_detect(const image &img, image &kptmap, buffer &kpts, buffer &numkpts,
                         int max_kpts, float thresh, float sigma) const
{
  float scale = 2.0f;
  gaussian_smooth_t gs = NEW_VISCL_TASK(gaussian_smooth);
  image smoothed = gs->smooth(img, scale, 2);
  detect(smoothed, kptmap, kpts, numkpts, max_kpts, thresh, sigma);
}

//*****************************************************************************

void hessian::detect(const image &smoothed, image &kptmap, buffer &kpts, buffer &numkpts,
                     int max_kpts, float thresh, float sigma) const
{
  size_t ni = smoothed.width(), nj = smoothed.height();
  cl::ImageFormat detimg_fmt(CL_INTENSITY, CL_FLOAT);
  image detimg = manager::inst()->create_image(detimg_fmt, CL_MEM_READ_WRITE, ni, nj);
  cl::ImageFormat kptimg_fmt(CL_R, CL_SIGNED_INT32);
  kptmap = manager::inst()->create_image(kptimg_fmt, CL_MEM_READ_WRITE, ni >> 1, nj >> 1);
  kpts = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_WRITE, max_kpts);

  int init[1];
  init[0] = 0;
  numkpts = manager::inst()->create_buffer<int>(CL_MEM_READ_WRITE, 1);
  queue->enqueueWriteBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), init);

  // Set arguments to kernel
  det_hessian->setArg(0, *smoothed().get());
  det_hessian->setArg(1, *detimg().get());
  det_hessian->setArg(2, sigma*sigma);

  init_kpt_map->setArg(0, *kptmap().get());

  // Set arguments to kernel
  detect_extrema->setArg(0, *detimg().get());
  detect_extrema->setArg(1, *kptmap().get());
  detect_extrema->setArg(2, *kpts().get());
  detect_extrema->setArg(3, *numkpts().get());
  detect_extrema->setArg(4, max_kpts);
  detect_extrema->setArg(5, thresh);

  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  cl::NDRange initsize(ni >> 1, nj >> 1);
  //cl::NDRange local(32,32);

  queue->enqueueNDRangeKernel(*det_hessian.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueNDRangeKernel(*init_kpt_map.get(), cl::NullRange, initsize, cl::NullRange);
  queue->enqueueBarrier();

  queue->enqueueNDRangeKernel(*detect_extrema.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();
}

//*****************************************************************************

int hessian::num_kpts(const buffer &numkpts_b)
{
  int buf[1];
  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  return buf[0];
}

}
