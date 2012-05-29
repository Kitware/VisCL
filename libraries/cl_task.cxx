#include "cl_task.h"
#include "cl_manager.h"

#include <boost/make_shared.hpp>

cl_task::cl_task(const char *source)
{
  program = cl_manager::inst()->build_source(source);
}

boost::shared_ptr<cl::Kernel> cl_task::make_kernel(const vcl_string &kernel_name)
{
  return boost::make_shared<cl::Kernel>(cl::Kernel(*program.get(), kernel_name.c_str()));
}
