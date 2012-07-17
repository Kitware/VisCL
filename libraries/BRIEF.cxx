/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "BRIEF.h"

#include <boost/make_shared.hpp>
#include <vcl_sstream.h>
#include <vcl_fstream.h>

#include "cl_task_registry.h"
#include "gaussian_smooth.h"
#include "cl_manager.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

extern const char* BRIEF_source;

//*****************************************************************************

template<int radius>
void brief<radius>::init()
{
  cl_task::build_source(generate_meta_source(BRIEF_source));
  brief_k = make_kernel("brief");
}

//*****************************************************************************

template<int radius>
void brief<radius>::init(const cl_program_t &prog)
{
  program = prog;
  brief_k = make_kernel("brief");
}

//*****************************************************************************

template<int radius>
vcl_string brief<radius>::generate_meta_source(const vcl_string &source)
{
  //Boost rand guarantees that the random values generating the comparison map for brief
  //will be constistent otherwise the descriptor algorithm would not be thread safe
  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist(-radius, radius);

  vcl_stringstream metasource;
  metasource << "__constant int4 map[128] = {";
  const unsigned int nsamples = 128;
  for (unsigned int i = 0; i < 128; i++)
  {
    int x1 = dist(gen);
    int y1 = dist(gen);
    int x2 = dist(gen);
    int y2 = dist(gen);
    metasource << "(int4)(" << x1 << "," << y1 << "," << x2 << "," << y2 << ")";
    if (i < nsamples-1)
      metasource << ",\n";
  }

  metasource << "};\n\n" << source;

  return metasource.str();
}

//*****************************************************************************

template<int radius>
cl_task_t brief<radius>::clone()
{
  type clone_ = boost::make_shared<brief<radius> >(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

template<int radius> template<class T>
void brief<radius>::compute_descriptors(const vil_image_view<T> &img, const vcl_vector<cl_int2> &kpts, vcl_vector<cl_int4> &descriptors, float sigma)
{
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  gaussian_smooth_t gs = NEW_VISCL_TASK(gaussian_smooth);
  cl_image smoothed_cl = gs->smooth(img_cl, sigma, 2);
  cl_buffer kpts_cl = cl_manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  cl_buffer descriptors_cl;
  compute_descriptors(smoothed_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}

//*****************************************************************************

template<int radius> template<class T>
void brief<radius>::compute_descriptors(const vil_image_view<T> &img, const vcl_vector<cl_int2> &kpts, vcl_vector<cl_int4> &descriptors)
{
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_buffer kpts_cl = cl_manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  cl_buffer descriptors_cl;
  compute_descriptors(img_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}

//*****************************************************************************

template<int radius>
void brief<radius>::compute_descriptors(const cl_image &img_s, const cl_buffer &kpts, size_t numkpts, cl_buffer &descriptors)
{
  descriptors = cl_manager::inst()->create_buffer<cl_int4>(CL_MEM_READ_WRITE, numkpts);
  brief_k->setArg(0, *img_s().get());
  brief_k->setArg(1, *kpts().get());
  brief_k->setArg(2, *descriptors().get());
  
  cl::NDRange global(numkpts);
  queue->enqueueNDRangeKernel(*brief_k.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();
}

//*****************************************************************************

//Should probably move this all to .h for implicit instantiation
template class brief<10>;
template void brief<10>::compute_descriptors(const vil_image_view<vxl_byte> &img, const vcl_vector<cl_int2> &kpts, vcl_vector<cl_int4> &descriptors, float sigma);
template void brief<10>::compute_descriptors(const vil_image_view<vxl_byte> &img, const vcl_vector<cl_int2> &kpts, vcl_vector<cl_int4> &descriptors);
