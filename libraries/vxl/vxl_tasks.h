/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_VXL_TASKS_H_
#define CL_VXL_TASKS_H_

#include <vxl/vxl_transfer.h>
#include <vil/vil_image_view.h>

#include <vcl_vector.h>


#include <viscl/core/image.h>
#include <viscl/core/manager.h>
#include <viscl/core/task_registry.h>
#include <viscl/tasks/hessian.h>
#include <viscl/tasks/BRIEF.h>
#include <viscl/tasks/gaussian_smooth.h>
#include <viscl/tasks/track_descr_match.h>


namespace viscl
{

template <class T>
void cl_hessian_detect(const vil_image_view<T> &img, int max_kpts, float thresh,
                       float sigma, vcl_vector<cl_int2> &kpts)
{
  image img_cl = upload_image(img);
  hessian_t hes = NEW_VISCL_TASK(hessian);
  image kptmap;
  buffer numkpts_b, kpts_b;
  hes->smooth_and_detect(img_cl, kptmap, kpts_b, numkpts_b, max_kpts, thresh, sigma);

  cl_queue_t queue = manager::inst()->create_queue();
  int buf[1];
  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  int numkpts = buf[0];

  kpts.resize(numkpts);
  queue->enqueueReadBuffer(*kpts_b().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);
}


//*****************************************************************************

template <class T>
void cl_gaussian_smooth(const vil_image_view<T> &img, vil_image_view<T> &output,
                        float sigma, int kernel_radius)
{
  image img_cl = upload_image(img);
  gaussian_smooth_t gs = NEW_VISCL_TASK(gaussian_smooth);
  image result = gs->smooth( img_cl, sigma, kernel_radius);

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(img.ni());
  region.push_back(img.nj());
  region.push_back(1);

  output.set_size(img.ni(), img.nj());
  cl_queue_t queue = manager::inst()->create_queue();
  queue->enqueueReadImage(*result().get(),  CL_TRUE, origin, region, 0, 0, (float *)output.top_left_ptr());
}

//*****************************************************************************

template<class pixtype>
void track_descr_first_frame(const vil_image_view<pixtype> &img,
                             vcl_vector<cl_int2> &kpts,
                             track_descr_match_t& tdm)
{
  image img_cl = upload_image(img);
  tdm->first_frame(img_cl);
  const buffer& kpts1 = tdm->last_keypoints();

  int numkpts = tdm->last_num_keypoints();
  kpts.clear();
  kpts.resize(numkpts);
  cl_queue_t queue = manager::inst()->create_queue();
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);
}

//*****************************************************************************

template<class pixtype>
vcl_vector<int> track_descr_track(const vil_image_view<pixtype> &img,
                                  vcl_vector<cl_int2> &kpts,
                                  int window_size,
                                  track_descr_match_t& tdm)
{
  image img_cl = upload_image(img);
  buffer tracks_b = tdm->track(img_cl, window_size);
  const buffer& kpts1 = tdm->last_keypoints();
  int numkpts = tdm->last_num_keypoints();

  vcl_vector<int> tracks;
  tracks.resize(numkpts);
  cl_queue_t queue = manager::inst()->create_queue();
  queue->enqueueReadBuffer(*tracks_b(), CL_TRUE, 0, tracks_b.mem_size(), &tracks[0]);

  kpts.clear();
  kpts.resize(numkpts);
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);

  return tracks;
}

//*****************************************************************************

template<class T, int R>
void compute_brief_descriptors(const vil_image_view<T> &img,
                               const vcl_vector<cl_int2> &kpts,
                               vcl_vector<cl_int4> &descriptors,
                               float sigma)
{
  image img_cl = upload_image(img);
  gaussian_smooth_t gs = NEW_VISCL_TASK(gaussian_smooth);
  image smoothed_cl = gs->smooth(img_cl, sigma, 2);
  buffer kpts_cl = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  cl_queue_t queue = manager::inst()->create_queue();
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  buffer descriptors_cl;
  typename brief<R>::type brf = NEW_VISCL_TASK(brief<R>);
  brf->compute_descriptors(smoothed_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}

//*****************************************************************************

template<class T, int R>
void compute_brief_descriptors(const vil_image_view<T> &img,
                               const vcl_vector<cl_int2> &kpts,
                               vcl_vector<cl_int4> &descriptors)
{
  image img_cl = upload_image(img);
  buffer kpts_cl = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  cl_queue_t queue = manager::inst()->create_queue();
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  buffer descriptors_cl;
  typename brief<R>::type brf = NEW_VISCL_TASK(brief<R>);
  brf->compute_descriptors(img_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}



} // end namespace viscl

#endif // CL_VXL_TASKS_H_
