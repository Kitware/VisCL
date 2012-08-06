/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_IMAGE_H_
#define CL_IMAGE_H_

#include "cl_header.h"
#include <boost/shared_ptr.hpp>

class cl_image
{
public:

  cl_image() {}
  cl_image(const cl_image_t &image);

  size_t width() const;
  size_t height() const;

  cl::ImageFormat format() const;

  const boost::shared_ptr<cl::Image2D> &operator()() const {return img;}

private:

  cl_image_t img;
};

#endif
