/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include "vxl_transfer.h"

#include <iostream>

#include <viscl/core/cl_manager.h>


//*****************************************************************************


//Does NOT support multiplane images or non-continuous memory
template<class T>
viscl::cl_image viscl::upload_image(const vil_image_view<T> &img)
{
  if (!img.top_left_ptr())
  {
    std::cerr << "No image data!\n";
  }

  cl::ImageFormat img_fmt;
  //http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/cl_image_format.html
  switch(img.pixel_format())
  {
    case VIL_PIXEL_FORMAT_FLOAT:
      img_fmt = cl::ImageFormat(CL_INTENSITY, CL_FLOAT);
      break;
    case VIL_PIXEL_FORMAT_BYTE:
      img_fmt = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);
      break;
    default:
      std::cerr << "unsupported vil image format" << std::endl;;
      return cl_image();
  }

  return cl_image(boost::make_shared<cl::Image2D>(cl::Image2D(cl_manager::inst()->get_context(),
                                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                  img_fmt,
                                                  img.ni(),
                                                  img.nj(),
                                                  0,
                                                  (T *)img.top_left_ptr())));
}


//*****************************************************************************

template viscl::cl_image viscl::upload_image<float>(const vil_image_view<float> &);
template viscl::cl_image viscl::upload_image<vxl_byte>(const vil_image_view<vxl_byte> &);
