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

#ifndef CL_HEADER_H_
#define CL_HEADER_H_

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
  #include <OpenCL/cl.hpp>
#else
  // Visual C++ doesn't support the "throws" syntax but can still work with
  // the rest of the exception handling.
  #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4290)
  #endif
  #include <CL/cl.hpp>
  #ifndef _MSC_VER
    #pragma warning(pop)
  #endif
#endif

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace viscl
{

typedef boost::shared_ptr<cl::CommandQueue> cl_queue_t;
typedef boost::shared_ptr<cl::Program> cl_program_t;
typedef boost::shared_ptr<cl::Kernel> cl_kernel_t;
typedef boost::shared_ptr<cl::Buffer> cl_buffer_t;
typedef boost::shared_ptr<cl::Image2D> cl_image_t;
typedef boost::shared_ptr<cl::Image3D> cl_image_3d_t;

}

#define NEW_VISCL_TASK(T) boost::make_shared<T>();

#endif
