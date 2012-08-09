/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_UTIL_H_
#define CL_UTIL_H_

#include <viscl/core/header.h>
#include <viscl/core/image.h>

namespace viscl
{

template<class T>
void save_cl_image(const cl_queue_t &queue, const image &img, const char *filename);
template<>
void save_cl_image<unsigned char>(const cl_queue_t &queue, const image &img, const char *filename);

} // end namespace viscl

#endif
