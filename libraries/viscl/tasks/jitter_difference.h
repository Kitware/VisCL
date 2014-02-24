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

#ifndef JITTER_DIFFERENCE_H_
#define JITTER_DIFFERENCE_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/buffer.h>


namespace viscl
{

class jitter_difference : public task
{
public:

  ///Make a jitter differencer with \jitter_delta_ and a locl work group size of
  ///local_x_ x local_y_
  jitter_difference(int jitter_delta_, int local_x_ = 10, int local_y_ = 10);

  //A and B are two previous frames, C is the current frame
  //Implements: diff = | jit(| A - C |) + jit(| C - B |) - jit(| A - B |) |
  //Assumes they are all the same size
  void diff(const image &A, const image &B, const image &C, const image &diff) const;

private:

  unsigned int local_x, local_y;
  jitter_difference() {}

  std::string generate_meta_source(const std::string &source);

  cl_kernel_t jitter_diff_k;
  int jitter_delta;
};

typedef boost::shared_ptr<jitter_difference> jitter_difference_t;

}

#endif
