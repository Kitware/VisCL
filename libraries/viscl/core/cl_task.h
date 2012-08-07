/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_TASK_H_
#define CL_TASK_H_

#include <string>

#include "cl_header.h"

namespace viscl
{

class cl_task;
typedef boost::shared_ptr<cl_task> cl_task_t;

class cl_task
{
public:

  virtual cl_task_t clone() = 0;

protected:

  friend class cl_task_registry;

  virtual void init() = 0;
  virtual void init(const cl_program_t &prog) = 0;

  cl_kernel_t make_kernel(const std::string &kernel_name);
  void build_source(const std::string &source);

  cl_program_t program;
};

}

#endif
