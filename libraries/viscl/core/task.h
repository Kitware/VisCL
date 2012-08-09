/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_TASK_H_
#define CL_TASK_H_

#include <string>

#include <viscl/core/header.h>

namespace viscl
{

class task;
typedef boost::shared_ptr<task> task_t;

class task
{
public:

  virtual task_t clone() = 0;
  cl_queue_t get_queue() const { return queue; }

protected:

  friend class task_registry;

  virtual void init() = 0;
  virtual void init(const cl_program_t &prog) = 0;

  cl_kernel_t make_kernel(const std::string &kernel_name);
  void build_source(const std::string &source);

  cl_program_t program;
  cl_queue_t queue;
};

}

#endif
