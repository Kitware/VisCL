/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_buffer.h"

namespace viscl
{

//*****************************************************************************

cl_buffer::cl_buffer(const boost::shared_ptr<cl::Buffer> &buffer, size_t length) : buf(buffer), len_(length)
{

}

//*****************************************************************************

size_t cl_buffer::mem_size() const
{
  size_t mem;
  buf->getInfo<size_t>(CL_MEM_SIZE, &mem);
  return mem;
}

//*****************************************************************************

}
