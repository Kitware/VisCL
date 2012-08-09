/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_descr_match.h"
#include <viscl/core/manager.h>

#include <viscl/core/task_registry.h>

#include <fstream>
#include <boost/make_shared.hpp>
#include <algorithm>
#include <iostream>


extern const char* track_descr_match_source;

namespace viscl
{

//*****************************************************************************

track_descr_match::track_descr_match()
{
  max_kpts = 5000;
}

//*****************************************************************************

void track_descr_match::init()
{
  task::build_source(track_descr_match_source);
  track_k = make_kernel("track");
}

//*****************************************************************************

void track_descr_match::init(const cl_program_t &prog)
{
  program = prog;
  track_k = make_kernel("track");
}

//*****************************************************************************

track_descr_match::~track_descr_match()
{
}

//*****************************************************************************

task_t track_descr_match::clone()
{
  track_descr_match_t clone_ = boost::make_shared<track_descr_match>(*this);
  clone_->queue = manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

//Copy constructor for cloning, does not clone tracks
track_descr_match::track_descr_match(const track_descr_match &t)
{
  this->program = t.program;
  this->track_k = t.track_k;
  this->max_kpts = t.max_kpts;
}

//*****************************************************************************

void track_descr_match::first_frame(const image &img)
{
  float thresh = 0.003f, sigma = 2.0f;
  gs = NEW_VISCL_TASK(gaussian_smooth);
  hes = NEW_VISCL_TASK(hessian);
  brf = NEW_VISCL_TASK(brief<10>);
  image smoothed = gs->smooth(img, sigma, 2);

  buffer numkpts_b;

  hes->detect(smoothed, kptmap1, kpts1, numkpts_b, max_kpts, thresh, sigma);
  numkpts1 = hes->num_kpts(numkpts_b);
  std::cout << numkpts1 << "\n";
  brf->compute_descriptors(smoothed, kpts1, numkpts1, descriptors1);
}

//*****************************************************************************

buffer track_descr_match::track(const image &img,
                                   int window_size)
{
  float thresh = 0.003f, sigma = 2.0f;
  image smoothed = gs->smooth(img, sigma, 2);

  buffer kpts2, numkpts2_b;
  image kptmap2;
  hes->detect(smoothed, kptmap2, kpts2, numkpts2_b, max_kpts, thresh, sigma);
  int numkpts2 = hes->num_kpts(numkpts2_b);
  std::cout << numkpts2 << "\n";

  buffer descriptors2;
  brf->compute_descriptors(smoothed, kpts2, numkpts2, descriptors2);

  buffer tracks_b = manager::inst()->create_buffer<int>(CL_MEM_WRITE_ONLY, numkpts2);

  track_k->setArg(0, *kpts2().get());
  track_k->setArg(1, *kptmap1().get());
  track_k->setArg(2, *descriptors1().get());
  track_k->setArg(3, *descriptors2().get());
  track_k->setArg(4, *tracks_b().get());
  track_k->setArg(5, window_size / 2);

  cl::NDRange global(numkpts2);
  queue->enqueueNDRangeKernel(*track_k.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();

  kpts1 = kpts2;
  kptmap1 = kptmap2;
  numkpts1 = numkpts2;
  descriptors1 = descriptors2;

  return tracks_b;
}

//*****************************************************************************

void write_tracks_to_file(const std::string& filename,
                          const std::vector<cl_int2> &kpts1,
                          const std::vector<cl_int2> &kpts2,
                          const std::vector<int> &indices)
{
  std::ofstream outfile(filename.c_str());
  for (unsigned int i = 0; i < indices.size(); i++)
  {
    if (indices[i] > -1)
    {
      const cl_int2& kpt1 = kpts1[indices[i]];
      const cl_int2& kpt2 = kpts2[i];
      outfile << kpt1.s[0] << " " << kpt1.s[1] << " "
              << kpt2.s[0] << " " << kpt2.s[1] << " "
              << indices[i] << "\n";
    }
  }
  outfile.close();
}

}
