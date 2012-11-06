/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <viscl/core/task.h>
#include <viscl/core/manager.h>

#include <fstream>
#include <boost/make_shared.hpp>

namespace viscl
{

boost::shared_ptr<cl::Kernel> task::make_kernel(const std::string &kernel_name)
{
  return boost::make_shared<cl::Kernel>(cl::Kernel(*program.get(), kernel_name.c_str()));
}

}
