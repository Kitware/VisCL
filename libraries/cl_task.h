#ifndef CL_TASK_H_
#define CL_TASK_H_

#include <vcl_vector.h>
#include <vcl_map.h>
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

class cl_task
{
public:

  cl_task() : program(NULL) {}

  void compile(const char *source);
  void add_kernel(const vcl_string &kernel_name);

  cl_task *clone();

  //Get the first kernel
  cl::Kernel *kernel() { return kernels.begin()->second; }

private:

  cl::Program *program;
  vcl_map<vcl_string, cl::Kernel *> kernels;

};

#endif
