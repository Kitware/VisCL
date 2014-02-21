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

#include "BRIEF.h"

#include <boost/make_shared.hpp>
#include <sstream>

#include <viscl/core/program_registry.h>
#include "gaussian_smooth.h"
#include <viscl/core/manager.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


extern const char* BRIEF_source;

namespace viscl
{

//*****************************************************************************

template<int radius>
brief<radius>::brief()
{
  //Create meta program name
  std::stringstream s;
  s << "brief " << radius;

  //Check if the meta program exists so we can avoid re-generating the src
  std::pair<cl_program_t, bool> p = program_registry::inst()->get_program(s.str());
  if (p.second)
    program = p.first;
  else
    program = program_registry::inst()->register_program(s.str(), generate_meta_source(BRIEF_source).c_str());

  brief_k = make_kernel("brief");
  queue = manager::inst()->create_queue();
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
