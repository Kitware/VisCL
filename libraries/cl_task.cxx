/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_task.h"
#include "cl_manager.h"

#include <boost/make_shared.hpp>

cl_task::cl_task(const vcl_string &source)
{
  vcl_ofstream outfile("foo.txt");
  outfile << source;
  outfile.close();
  program = cl_manager::inst()->build_source(source.c_str());
}

boost::shared_ptr<cl::Kernel> cl_task::make_kernel(const vcl_string &kernel_name)
{
  return boost::make_shared<cl::Kernel>(cl::Kernel(*program.get(), kernel_name.c_str()));
}
