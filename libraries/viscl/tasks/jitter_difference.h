/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
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

  jitter_difference(int jitter_delta_);
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
