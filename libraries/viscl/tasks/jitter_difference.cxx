/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "jitter_difference.h"

#include <boost/make_shared.hpp>

#include <viscl/core/program_registry.h>
#include <viscl/core/manager.h>
#include <sstream>

extern const char* jitter_difference_source;

namespace viscl
{

//*****************************************************************************

jitter_difference::jitter_difference(int jitter_delta_, int local_x_, int local_y_)
  : local_x(local_x_), local_y(local_y_), jitter_delta(jitter_delta_)
{
  //Create meta program name
  std::stringstream s;
  s << "jitter_difference " << jitter_delta << " " << local_x << " " << local_y;

  //Check if the meta program exists so we can avoid re-generating the src
  std::pair<cl_program_t, bool> p = program_registry::inst()->get_program(s.str());
  if (p.second)
    program = p.first;
  else
    program = program_registry::inst()->register_program(s.str(), generate_meta_source(jitter_difference_source).c_str());

  jitter_diff_k = make_kernel("jitter_difference");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

std::string jitter_difference::generate_meta_source(const std::string &source)
{
  std::stringstream metasource;
  metasource << "#define JITTER_DELTA " << jitter_delta << "\n";
  metasource << "#define JITTER_DELTA_X2 " << 2*jitter_delta << "\n";
  metasource << "#define LOCAL_X " << local_x << "\n";
  metasource << "#define LOCAL_Y " << local_y << "\n\n";
  metasource << source;
  return metasource.str();
}

//*****************************************************************************

//A and B are two previous frames, C is the current frame
//All images should be of the same size
void jitter_difference::diff(const image &A, const image &B, const image &C, const image &diff) const
{
  jitter_diff_k->setArg(0, *A().get());
  jitter_diff_k->setArg(1, *B().get());
  jitter_diff_k->setArg(2, *C().get());
  jitter_diff_k->setArg(3, *diff().get());

  queue->enqueueNDRangeKernel(*jitter_diff_k.get(), cl::NullRange,
                              cl::NDRange(diff.width(), diff.height()), cl::NDRange(local_x, local_y));
  queue->finish();
}

//****************************************************************************

}
