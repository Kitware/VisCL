/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <viscl/core/image_3d.h>

namespace viscl
{


image_3d::image_3d(const cl_image_3d_t &image) : img(image)
{

}

//*****************************************************************************

size_t image_3d::width() const
{
  size_t width;
  img->getImageInfo<size_t>(CL_IMAGE_WIDTH, &width);
  return width;
}

//*****************************************************************************

size_t image_3d::height() const
{
  size_t height;
  img->getImageInfo<size_t>(CL_IMAGE_HEIGHT, &height);
  return height;
}

//*****************************************************************************

size_t image_3d::depth() const
{
  size_t depth;
  img->getImageInfo<size_t>(CL_IMAGE_DEPTH, &depth);
  return depth;
}


//*****************************************************************************

cl::ImageFormat image_3d::format() const
{
  cl::ImageFormat imgf;
  img->getImageInfo<cl::ImageFormat>(CL_IMAGE_FORMAT, &imgf);
  return imgf;
}

//*****************************************************************************

}
