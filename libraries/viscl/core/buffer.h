/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_BUFFER_H_
#define CL_BUFFER_H_

#include <viscl/core/header.h>
#include <boost/shared_ptr.hpp>

namespace viscl
{

class buffer
{
public:

  buffer() {}
  buffer(const cl_buffer_t &buffer, size_t length);

  size_t mem_size() const;
  size_t len() const { return len_; }

  const boost::shared_ptr<cl::Buffer> &operator()() const {return buf;}

private:

  cl_buffer_t buf;
  size_t len_;
};

}

#endif
