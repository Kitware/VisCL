/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_task.h"
#include "cl_manager.h"

#include <fstream>
#include <boost/make_shared.hpp>

namespace viscl
{

boost::shared_ptr<cl::Kernel> cl_task::make_kernel(const std::string &kernel_name)
{
  return boost::make_shared<cl::Kernel>(cl::Kernel(*program.get(), kernel_name.c_str()));
}

void cl_task::build_source(const std::string &source)
{
  program = cl_manager::inst()->build_source(source.c_str());
}

}