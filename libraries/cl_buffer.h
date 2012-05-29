#ifndef CL_BUFFER_H_
#define CL_BUFFER_H_

#include "cl_header.h"
#include <boost/shared_ptr.hpp>

class cl_buffer
{
public:

  cl_buffer() {}
  cl_buffer(const cl_buffer_t &buffer, size_t length);

  size_t mem_size() const;
  size_t len() const { return len_; }

  const boost::shared_ptr<cl::Buffer> &operator()() const {return buf;}

private:

  cl_buffer_t buf;
  size_t len_;
};

#endif
