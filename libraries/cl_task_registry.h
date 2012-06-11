#ifndef CL_TASK_REGISTRY
#define CL_TASK_REGISTRY

#include "cl_task.h"
#include <boost/make_shared.hpp>

#define NEW_VISCL_TASK(T) cl_task_registry::inst()->new_task<T>(vcl_string(#T))

class cl_task_registry
{
public:

  static cl_task_registry *inst();

  template<class T>
  boost::shared_ptr<T> new_task(const vcl_string &task_name)
  {
    vcl_map<vcl_string, cl_task_t>::const_iterator itr = tasks.find(task_name);
    if (itr != tasks.end())
      return boost::dynamic_pointer_cast<T>(itr->second->clone());
    else
    {
      boost::shared_ptr<T> base(new T);
      tasks.insert(std::make_pair(task_name, boost::static_pointer_cast<cl_task>(base)));
      return boost::dynamic_pointer_cast<T>(base->clone());
    }
  }

  //Can be called to compile/initialize a task before making one
  template<class T>
  void init_task(const vcl_string &task_name)
  {
    vcl_map<vcl_string, cl_task_t>::const_iterator itr = tasks.find(task_name);
    if (itr == tasks.end())
    {
      boost::shared_ptr<T> base(new T);
      tasks.insert(std::make_pair(task_name, boost::dynamic_pointer_cast<cl_task>(base)));
    }
  }

private:

  cl_task_registry() { }
  static cl_task_registry *inst_;

  vcl_map<vcl_string, cl_task_t> tasks;
};

#endif
