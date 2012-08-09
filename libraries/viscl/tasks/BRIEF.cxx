/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "BRIEF.h"

#include <boost/make_shared.hpp>
#include <sstream>

#include <viscl/core/task_registry.h>
#include "gaussian_smooth.h"
#include <viscl/core/manager.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


extern const char* BRIEF_source;

namespace viscl
{

//*****************************************************************************

template<int radius>
void brief<radius>::init()
{
  task::build_source(generate_meta_source(BRIEF_source));
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
std::string brief<radius>::generate_meta_source(const std::string &source)
{
  //Boost rand guarantees that the random values generating the comparison map for brief
  //will be constistent otherwise the descriptor algorithm would not be thread safe
  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist(-radius, radius);

  std::stringstream metasource;
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
task_t brief<radius>::clone()
{
  type clone_ = boost::make_shared<brief<radius> >(*this);
  clone_->queue = manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

template<int radius>
void brief<radius>::compute_descriptors(const image &img_s, const buffer &kpts, size_t numkpts, buffer &descriptors)
{
  descriptors = manager::inst()->create_buffer<cl_int4>(CL_MEM_READ_WRITE, numkpts);
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

}
