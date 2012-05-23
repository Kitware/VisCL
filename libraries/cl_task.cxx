#include "cl_task.h"
#include "cl_manager.h"

void cl_task::compile(const char *source)
{
  program = cl_manager::inst()->build_source(source);
}

void cl_task::add_kernel(const vcl_string &kernel_name)
{
  if (program)
    kernels[kernel_name] =  new cl::Kernel(*program, kernel_name.c_str());
}

cl_task *cl_task::clone()
{
  cl_task *cloned = new cl_task;
  cloned->program = program;
  kernels = kernels;
  return cloned;
}
