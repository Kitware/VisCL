#include "cl_task_registry.h"

#include "gaussian_smooth.h"
#include "hessian.h"

cl_task_registry *cl_task_registry::inst_ = 0;

cl_task_registry *cl_task_registry::inst()
{
  return inst_ ? inst_ : inst_ = new cl_task_registry;
}

cl_task_registry::cl_task_registry()
{
  tasks["gaussian_smooth"] = new gaussian_smooth;
  tasks["hessian"] = new hessian;
}

cl_task *cl_task_registry::new_task(const vcl_string &task_name)
{
  return tasks[task_name]->clone();
}

