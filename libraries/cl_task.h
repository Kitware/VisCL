#ifndef CL_TASK_H_
#define CL_TASK_H_

#include <vcl_vector.h>
#include <vcl_map.h>
#include <vcl_string.h>

#include "cl_header.h"

class cl_task;
typedef boost::shared_ptr<cl_task> cl_task_t;

class cl_task
{
public:

  cl_task(const vcl_string &source);

  cl_kernel_t make_kernel(const vcl_string &kernel_name);

  virtual cl_task_t clone() = 0;

protected:

  cl_program_t program;
  cl_task() {}
};


#endif
