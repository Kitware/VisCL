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


#include "transfer.h"

#include <iostream>

#include <viscl/core/manager.h>


//*****************************************************************************

//Does NOT support multiplane images or non-continuous memory
template<class T>
viscl::image viscl::upload_image(const vil_image_view<T> &img)
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
      img_fmt = cl::ImageFormat(CL_R, CL_UNSIGNED_INT8);
      break;
    default:
      std::cerr << "unsupported vil image format" << std::endl;;
      return viscl::image();
  }

  return viscl::image(boost::make_shared<cl::Image2D>(
            cl::Image2D(viscl::manager::inst()->get_context(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            img_fmt,
            img.ni(),
            img.nj(),
            0,
            (T *)img.top_left_ptr())));
}

//*****************************************************************************

template<class T>
void viscl::download_image(const viscl::image &img_cl, vil_image_view<T> &img, const viscl::cl_queue_t &queue)
{
  size_t width = img_cl.width();
  size_t height = img_cl.height();
  img.set_size(width, height, 1);

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  if (queue)
    queue->enqueueReadImage(*img_cl().get(), CL_TRUE, origin, region,
                          0, 0, img.top_left_ptr());
  else
  {
    viscl::cl_queue_t q = viscl::manager::inst()->create_queue();
    q->enqueueReadImage(*img_cl().get(), CL_TRUE, origin, region,
                            0, 0, img.top_left_ptr());
  }
}

//*****************************************************************************

template viscl::image viscl::upload_image<float>(const vil_image_view<float> &);
template viscl::image viscl::upload_image<vxl_byte>(const vil_image_view<vxl_byte> &);

template void viscl::download_image(const viscl::image &img_cl, vil_image_view<float> &img, const viscl::cl_queue_t &queue);
template void viscl::download_image(const viscl::image &img_cl, vil_image_view<vxl_byte> &img, const viscl::cl_queue_t &queue);

//*****************************************************************************
