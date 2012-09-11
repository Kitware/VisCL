/*ckwg +29
 * Copyright 2012 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "track_descr_match.h"
#include <viscl/core/manager.h>

#include <viscl/core/program_registry.h>

#include <fstream>
#include <boost/make_shared.hpp>
#include <algorithm>
#include <iostream>


extern const char* track_descr_match_source;

namespace viscl
{

//*****************************************************************************

track_descr_match::track_descr_match()
 : max_kpts_(5000),
   search_box_radius_(50),
   hamming_dist_threshold_(15),
   detect_thresh_(0.003f),
   smooth_sigma_(2.0f),
   subpixel_(false),
   kpts_buffer_size_(0)
{
  program = program_registry::inst()->register_program(std::string("track_descr_match"),
                                                       track_descr_match_source);
  track_k = make_kernel("track");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

track_descr_match::~track_descr_match()
{

}

//*****************************************************************************

void track_descr_match::first_frame(const image &img)
{
  gs = NEW_VISCL_TASK(viscl::gaussian_smooth);
  hes = NEW_VISCL_TASK(viscl::hessian);
  brf = NEW_VISCL_TASK(viscl::brief<10>);
  image smoothed = gs->smooth(img, smooth_sigma_, 2);

  buffer numkpts_b, kvals;

  const size_t ni = img.width(), nj = img.height();
  // a hard upper bound on the number of keypoints that can be detected
  const unsigned max_kpts = ni * nj / 4;

  hes->detect(smoothed, kptmap1, kpts1, kvals, numkpts_b,
              detect_thresh_, smooth_sigma_, subpixel_);
  numkpts1 = hes->num_kpts(numkpts_b);
  // allocate 1.5x as much memory for the next frame to provided a buffer.
  kpts_buffer_size_ = std::min(3*numkpts1/2, max_kpts);
  std::cout << numkpts1 << "\n";
  brf->compute_descriptors(smoothed, kpts1, numkpts1, descriptors1);
}

//*****************************************************************************

buffer track_descr_match::track(const image &img)
{
  image smoothed = gs->smooth(img, smooth_sigma_, 2);

  const size_t ni = img.width(), nj = img.height();
  // a hard upper bound on the number of keypoints that can be detected
  const unsigned max_kpts = ni * nj / 4;

  buffer kpts2, kvals, numkpts2_b;
  image kptmap2;
  kpts2 = manager::inst()->create_buffer<cl_float2>(CL_MEM_READ_WRITE, kpts_buffer_size_);
  kvals = manager::inst()->create_buffer<cl_float>(CL_MEM_READ_WRITE, kpts_buffer_size_);

  hes->detect(smoothed, kptmap2, kpts2, kvals, numkpts2_b,
              detect_thresh_, smooth_sigma_, subpixel_);
  unsigned numkpts2 = hes->num_kpts(numkpts2_b);
  std::cout << numkpts2 << "\n";
  if (numkpts2 > kpts_buffer_size_)
  {
    // allocate 1.5x as much memory for the next frame to provided a buffer.
    kpts_buffer_size_ = std::min(3*numkpts2/2, max_kpts);
  }

  buffer descriptors2;
  brf->compute_descriptors(smoothed, kpts2, numkpts2, descriptors2);

  buffer tracks_b = manager::inst()->create_buffer<int>(CL_MEM_WRITE_ONLY, numkpts2);

  track_k->setArg(0, *kpts2().get());
  track_k->setArg(1, *kptmap1().get());
  track_k->setArg(2, *descriptors1().get());
  track_k->setArg(3, *descriptors2().get());
  track_k->setArg(4, *tracks_b().get());
  track_k->setArg(5, search_box_radius_);
  track_k->setArg(6, hamming_dist_threshold_);

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
                          const std::vector<cl_float2> &kpts1,
                          const std::vector<cl_float2> &kpts2,
                          const std::vector<int> &indices)
{
  std::ofstream outfile(filename.c_str());
  for (unsigned int i = 0; i < indices.size(); i++)
  {
    if (indices[i] > -1)
    {
      const cl_float2& kpt1 = kpts1[indices[i]];
      const cl_float2& kpt2 = kpts2[i];
      outfile << kpt1.s[0] << " " << kpt1.s[1] << " "
              << kpt2.s[0] << " " << kpt2.s[1] << " "
              << indices[i] << "\n";
    }
  }
  outfile.close();
}

//*****************************************************************************

}
