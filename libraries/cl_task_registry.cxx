#include "cl_task_registry.h"

cl_task_registry *cl_task_registry::inst_ = 0;

cl_task_registry *cl_task_registry::inst()
{
  return inst_ ? inst_ : inst_ = new cl_task_registry;
}
