#ifndef CL_HEADER_H_
#define CL_HEADER_H_

#include <vcl_vector.h>
#include <vcl_string.h>

#define __NO_STD_VECTOR
#define __USE_DEV_VECTOR
#define VECTOR_CLASS vcl_vector

#define __NO_STD_STRING
#define __USE_DEV_STRING
typedef vcl_string STRING_CLASS;

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<cl::CommandQueue> cl_queue_t;
typedef boost::shared_ptr<cl::Program> cl_program_t;
typedef boost::shared_ptr<cl::Kernel> cl_kernel_t;
typedef boost::shared_ptr<cl::Buffer> cl_buffer_t;
typedef boost::shared_ptr<cl::Image2D> cl_image_t;

#endif
