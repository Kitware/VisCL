#ifndef CL_MANAGER_H_
#define CL_MANAGER_H_

#include <vcl_vector.h>
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

class cl_manager
{
public:

  static cl_manager *inst();
  
  void init_opencl();

  const cl::Context &get_context() const { return context; }
  const cl::Device &get_device(int device = 0) const { return devices[device]; }
  
  cl::Program *build_source(const char *source, int device = 0) const;
  cl::CommandQueue *create_queue(int device = 0);
  
private:

  cl_manager();
  static cl_manager *inst_;

  vcl_vector<cl::Platform> platforms;
  cl::Context context;
  vcl_vector<cl::Device> devices;
};

const char *print_cl_errstring(cl_int err);

#endif