/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef BRIEF_H_
#define BRIEF_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/buffer.h>

namespace viscl
{

template<int radius>
class brief : public task
{
public:

  brief();
  typedef boost::shared_ptr<brief<radius> > type;

  void compute_descriptors(const image &img_s, const buffer &kpts, size_t numkpts, buffer &descriptors);

private:

  std::string generate_meta_source(const std::string &source);

  cl_kernel_t brief_k, brief_dist_k;
};

}

#endif
