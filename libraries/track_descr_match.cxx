/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_descr_match.h"
#include "cl_manager.h"

#include "cl_task_registry.h"

#include <boost/make_shared.hpp>

extern const char* track_descr_match_source;

track_descr_match::track_descr_match() : cl_task(track_descr_match_source)
{
  track_k = make_kernel("track");
  max_kpts = 5000;
}

cl_task_t track_descr_match::clone()
{
  track_descr_match_t clone_ = boost::make_shared<track_descr_match>(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//Copy constructor for cloning, does not clone tracks
track_descr_match::track_descr_match(const track_descr_match &t)
{
  this->program = t.program;
  this->track_k = t.track_k;
  this->max_kpts = t.max_kpts;
}

template<class T>
void track_descr_match::first_frame(const vil_image_view<T> &img)
{
  float thresh = 0.007f, sigma = 2.0f;
  gs = NEW_VISCL_TASK(gaussian_smooth);
  hes = NEW_VISCL_TASK(hessian);
  brf = NEW_VISCL_TASK(brief<10>);
  vcl_cout << "start\n";

  cl_image img_cl = cl_manager::inst()->create_image<T>(img);

  cl_image smoothed = gs->smooth(img_cl, sigma);
  img_cl.del();

  cl_buffer numkpts_b;
  cl_image kptmap_first;

  hes->detect(smoothed, kptmap_first, kpts1, numkpts_b, max_kpts, thresh, sigma);
  numkpts1 = hes->num_kpts(numkpts_b);

  brf->compute_descriptors(smoothed, kpts1, numkpts1, descriptors1);
}

template<class T>
vcl_vector<tdm_track> track_descr_match::track(const vil_image_view<T> &img, int window_size)
{
  float thresh = 0.007f, sigma = 2.0f;
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_image smoothed = gs->smooth(img_cl, sigma);
  img_cl.del(); 
  
  cl_buffer kpts2, numkpts2_b;
  cl_image kptmap;
  hes->detect(smoothed, kptmap, kpts2, numkpts2_b, max_kpts, thresh, sigma);
  int numkpts2 = hes->num_kpts(numkpts2_b);

  cl_buffer descriptors2;
  brf->compute_descriptors(smoothed, kpts2, numkpts2, descriptors2);

  cl_buffer tracks_b = cl_manager::inst()->create_buffer<int>(CL_MEM_WRITE_ONLY, numkpts1);

  track_k->setArg(0, *kpts1().get());
  track_k->setArg(1, *kptmap().get());
  track_k->setArg(2, *descriptors1().get());
  track_k->setArg(3, *descriptors2().get());
  track_k->setArg(4, *tracks_b().get());
  track_k->setArg(5, window_size / 2);

  cl::NDRange global(numkpts1);
  queue->enqueueNDRangeKernel(*track_k.get(), cl::NullRange, global, cl::NullRange);

  int *indices = new int[numkpts1];
  queue->enqueueReadBuffer(*tracks_b(), CL_TRUE, 0, tracks_b.mem_size(), indices);

  vcl_vector<cl_int2> kpts1_v(numkpts1), kpts2_v(numkpts2);
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts1, &kpts1_v[0]);
  queue->enqueueReadBuffer(*kpts2().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts2, &kpts2_v[0]);

  tracks.reserve(numkpts1);
  for (int i = 0; i < numkpts1; i++)
  {
    if (indices[i] > -1) 
    {
      pt_track tr;
      tr.pt_prev = vnl_int_2(kpts1_v[i].s[0], kpts1_v[i].s[1]);
      tr.pt_new = vnl_int_2(kpts2_v[indices[i]].s[0], kpts2_v[indices[i]].s[1]);
      tracks.push_back(tr);
    }
  }

  kpts1 = kpts2;
  numkpts1 = numkpts2;
  descriptors1 = descriptors2;
  delete [] indices;

  return tracks;
}

void track_descr_match::write_tracks_to_file(const char *filename)
{
  vcl_ofstream outfile("tracks.txt");
  for (int i = 0; i < tracks.size(); i++)
  {
    outfile << tracks[i].pt_prev[0] << " " << tracks[i].pt_prev[1] << " "
            << tracks[i].pt_new[0] << " " << tracks[i].pt_new[1] << "\n";
  }
  outfile.close();
}

template void track_descr_match::first_frame(const vil_image_view<vxl_byte> &img);
template vcl_vector<tdm_track> track_descr_match::track(const vil_image_view<vxl_byte> &img, int window_size);

