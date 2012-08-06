/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_HEADER_H_
#define CL_HEADER_H_

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
typedef boost::shared_ptr<cl::Image3D> cl_image_3d_t;

#endif
