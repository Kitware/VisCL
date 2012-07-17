/*ckwg +5
 * Copyright 2010 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_descr_match.h"
#include "cl_manager.h"

#include "cl_task_registry.h"

#include <fstream>
#include <boost/make_shared.hpp>
#include <vcl_algorithm.h>

extern const char* track_descr_match_source;

//*****************************************************************************

track_descr_match::track_descr_match()
{
  max_kpts = 5000;
}

//*****************************************************************************

void track_descr_match::init()
{
  cl_task::build_source(track_descr_match_source);
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
  delete kpts1_v;
  delete kpts2_v;
}

//*****************************************************************************

cl_task_t track_descr_match::clone()
{
  track_descr_match_t clone_ = boost::make_shared<track_descr_match>(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  clone_->kpts1_v = new vcl_vector<cl_int2>;
  clone_->kpts2_v = new vcl_vector<cl_int2>;
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

template<class pixtype, class loctype>
void track_descr_match::first_frame(const vil_image_view<pixtype> &img,
                                    vcl_vector<vnl_vector_fixed<loctype, 2> > &kpts)
{
  float thresh = 0.003f, sigma = 2.0f;
  gs = NEW_VISCL_TASK(gaussian_smooth);
  hes = NEW_VISCL_TASK(hessian);
  brf = NEW_VISCL_TASK(brief<10>);
  cl_image smoothed;

  {
    cl_image img_cl = cl_manager::inst()->create_image<pixtype>(img);
    smoothed = gs->smooth(img_cl, sigma, 2);
  }

  cl_buffer numkpts_b;

  hes->detect(smoothed, kptmap1, kpts1, numkpts_b, max_kpts, thresh, sigma);
  numkpts1 = hes->num_kpts(numkpts_b);
  vcl_cout << numkpts1 << "\n";
  brf->compute_descriptors(smoothed, kpts1, numkpts1, descriptors1);

  kpts1_v->resize(numkpts1);
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts1, &(*kpts1_v)[0]);

  kpts.reserve(numkpts1);
  for (int i = 0; i < numkpts1; i++)
  {
    const cl_int2 &loc = (*kpts1_v)[i];
    kpts.push_back(vnl_vector_fixed<loctype, 2>((loctype)loc.s[0],(loctype)loc.s[1]));
  }
}

//*****************************************************************************

template<class pixtype, class loctype>
const vcl_vector<int>& track_descr_match::track(const vil_image_view<pixtype> &img,
                               vcl_vector<vnl_vector_fixed<loctype, 2> > &kpts,
                               int window_size)
{
  float thresh = 0.003f, sigma = 2.0f;
  cl_image smoothed;

  {
    cl_image img_cl = cl_manager::inst()->create_image<pixtype>(img);
    smoothed = gs->smooth(img_cl, sigma, 2);
  }

  cl_buffer kpts2, numkpts2_b;
  cl_image kptmap2;
  hes->detect(smoothed, kptmap2, kpts2, numkpts2_b, max_kpts, thresh, sigma);
  int numkpts2 = hes->num_kpts(numkpts2_b);
  vcl_cout << numkpts2 << "\n";

  cl_buffer descriptors2;
  brf->compute_descriptors(smoothed, kpts2, numkpts2, descriptors2);

  cl_buffer tracks_b = cl_manager::inst()->create_buffer<int>(CL_MEM_WRITE_ONLY, numkpts2);

  track_k->setArg(0, *kpts2().get());
  track_k->setArg(1, *kptmap1().get());
  track_k->setArg(2, *descriptors1().get());
  track_k->setArg(3, *descriptors2().get());
  track_k->setArg(4, *tracks_b().get());
  track_k->setArg(5, window_size / 2);

  cl::NDRange global(numkpts2);
  queue->enqueueNDRangeKernel(*track_k.get(), cl::NullRange, global, cl::NullRange);

  tracks.resize(numkpts2);
  queue->enqueueReadBuffer(*tracks_b(), CL_TRUE, 0, tracks_b.mem_size(), &tracks[0]);

  kpts2_v->resize(numkpts2);
  queue->enqueueReadBuffer(*kpts2().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts2, &(*kpts2_v)[0]);

  kpts.reserve(numkpts2);
  for (int i = 0; i < numkpts2; i++)
  {
    const cl_int2 &loc = (*kpts2_v)[i];
    kpts.push_back(vnl_vector_fixed<loctype, 2>((loctype)loc.s[0],(loctype)loc.s[1]));
  }

  kpts1 = kpts2;
  kptmap1 = kptmap2;
  numkpts1 = numkpts2;
  descriptors1 = descriptors2;
  vcl_swap(kpts1_v, kpts2_v);

  return tracks;
}

//*****************************************************************************

template<class T>
void write_tracks_to_file(const char *filename, const vcl_vector<vnl_vector_fixed<T, 2> > &kpts1,
                          const vcl_vector<vnl_vector_fixed<T, 2> > &kpts2, const vcl_vector<int> &indices)
{
  vcl_ofstream outfile(filename);
  for (unsigned int i = 0; i < indices.size(); i++)
  {
    if (indices[i] > -1)
    {
      outfile << kpts1[indices[i]] << " " << kpts2[i] << " " << indices[i] << "\n";
    }
  }
  outfile.close();
}

//*****************************************************************************

template void track_descr_match::first_frame<vxl_byte, double>(const vil_image_view<vxl_byte> &img,
                                                               vcl_vector<vnl_vector_fixed<double, 2> > &kpts);
template const vcl_vector<int>& track_descr_match::track(const vil_image_view<vxl_byte> &img,
                                                         vcl_vector<vnl_vector_fixed<double, 2> > &kpts,
                                                         int window_size);
template void write_tracks_to_file(const char *filename, const vcl_vector<vnl_vector_fixed<double, 2> >  &kpts1,
                          const vcl_vector<vnl_vector_fixed<double, 2> > &kpts2, const vcl_vector<int> &indices);
