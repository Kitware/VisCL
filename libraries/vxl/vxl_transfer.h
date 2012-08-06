/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_VXL_TRANSFER_H_
#define CL_VXL_TRANSFER_H_

#include <viscl/core/cl_image.h>

#include <vil/vil_image_view.h>


namespace viscl
{

//Does NOT support multiplane images or non-continuous memory
template<class T>
cl_image upload_image(const vil_image_view<T> &img);


} // end namespace viscl

#endif // CL_VXL_TRANSFER_H_
