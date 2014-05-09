/*ckwg +29
 * Copyright 2012-2014 by Kitware, Inc.
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

#ifndef CL_MANAGER_H_
#define CL_MANAGER_H_

#include <viscl/core/header.h>

#include <boost/make_shared.hpp>

#include <map>
#include <vector>

#include <viscl/core/image.h>
#include <viscl/core/buffer.h>

#include "config.h"

namespace viscl
{

class VISCL_EXPORT manager
{
public:

  static manager *inst();

  const cl::Context &get_context() const { return context_; }
  const cl::Device &get_device(int device = 0) const { return devices_[device]; }

  cl_program_t build_source(const char *source, int device = 0) const;
  cl_queue_t create_queue(int device = 0);

  image create_image(const cl::ImageFormat &img_frmt, cl_mem_flags flags, size_t ni, size_t nj);
  //template<class T>
  //buffer create_buffer(T *, cl_mem_flags flags, size_t len);
  template<class T>
  buffer create_buffer(cl_mem_flags flags, size_t len);

  /// Print specs for a given device number on the active Platform.
  void report_device_specs(int device = 0)
  {
    report_device_specs(devices_[device]);
  }

  /// Print specs for all available OpenCL Platforms and Devices.
  void report_opencl_specs();

private:

  /// Print specs for an OpenCL Device.
  /// Prefix each output line with \a prefix
  void report_device_specs(const cl::Device& dev,
                           const std::string& prefix="");

  manager();
  static manager *inst_;

  void init_opencl();

  std::vector<cl::Platform> platforms_;
  cl::Context context_;
  std::vector<cl::Device> devices_;
};

VISCL_EXPORT  const char *print_cl_errstring(cl_int err);

//Implicit Instantiations
template<class T>
buffer manager::create_buffer(cl_mem_flags flags, size_t len)
{
  return buffer(boost::make_shared<cl::Buffer>(cl::Buffer(context_, flags, len * sizeof(T))), len);
}

}

#endif
