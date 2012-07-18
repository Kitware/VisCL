/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_MANAGER_H_
#define CL_MANAGER_H_

#include "cl_header.h"

#include <boost/make_shared.hpp>

#include <vil/vil_image_view.h>
#include <vcl_map.h>

#include "cl_image.h"
#include "cl_buffer.h"

class cl_manager
{
public:

  static cl_manager *inst();

  const cl::Context &get_context() const { return context_; }
  const cl::Device &get_device(int device = 0) const { return devices_[device]; }

  cl_program_t build_source(const char *source, int device = 0) const;
  cl_queue_t create_queue(int device = 0);

  //This function will reformat image views to interleaved
  template<class T>
  cl_image create_image(const vil_image_view<T> &img);
  cl_image create_image(const cl::ImageFormat &img_frmt, cl_mem_flags flags, size_t ni, size_t nj);
  //template<class T>
  //cl_buffer create_buffer(T *, cl_mem_flags flags, size_t len);
  template<class T>
  cl_buffer create_buffer(cl_mem_flags flags, size_t len);

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
                           const vcl_string& prefix="");

  cl_manager();
  static cl_manager *inst_;

  void init_opencl();
  void make_pixel_format_map();

  vcl_vector<cl::Platform> platforms_;
  cl::Context context_;
  vcl_vector<cl::Device> devices_;

  vcl_map<vil_pixel_format, cl::ImageFormat> pixel_format_map_;
};

const char *print_cl_errstring(cl_int err);

//Implicit Instantiations
template<class T>
cl_buffer cl_manager::create_buffer(cl_mem_flags flags, size_t len)
{
  return cl_buffer(boost::make_shared<cl::Buffer>(cl::Buffer(context_, flags, len * sizeof(T))), len);
}

#endif
