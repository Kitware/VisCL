#include "cl_buffer.h"

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

