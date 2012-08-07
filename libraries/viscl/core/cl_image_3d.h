/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_IMAGE_3D_H_
#define CL_IMAGE_3D_H_

#include "cl_header.h"
#include <boost/shared_ptr.hpp>

namespace viscl
{

class cl_image_3d
{
public:

  cl_image_3d() {}
  cl_image_3d(const cl_image_3d_t &image);

  size_t width() const;
  size_t height() const;
  size_t depth() const;

  cl::ImageFormat format() const;

  const cl_image_3d_t &operator()() const {return img;}

private:

  cl_image_3d_t img;
};

}

#endif
