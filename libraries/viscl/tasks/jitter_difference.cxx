/*ckwg +29
 * Copyright 2012-2014 by Kitware, Inc.
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
