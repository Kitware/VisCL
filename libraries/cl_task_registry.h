#ifndef CL_TASK_REGISTRY
#define CL_TASK_REGISTRY

/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_task.h"
#include <vcl_map.h>
#include <boost/make_shared.hpp>

//Cloning a task does not deep copy its kernel, therefore if a task is used in multiple threads 
//there will be a race-condition for the kernel args.  Thus deep_clone should be used when
//using a kernel in a different thread

#define NEW_VISCL_TASK(T) cl_task_registry::inst()->new_task<T>(vcl_string(#T), 0)
#define NEW_THREAD_SAFE_VISCL_TASK(T, thread_id) cl_task_registry::inst()->new_task<T>(vcl_string(#T), (thread_id));

class cl_task_registry
{
public:

  static cl_task_registry *inst();

  template<class T>
  boost::shared_ptr<T> new_task(const vcl_string &task_name, unsigned int thread_id)
  {
    vcl_map<vcl_string, vcl_map<int, cl_task_t> >::iterator p = tasks.find(task_name);
    if (p != tasks.end())
    {
      vcl_map<int, cl_task_t>::const_iterator q = p->second.find(thread_id);
      if (q != p->second.end())
      {
        return boost::dynamic_pointer_cast<T>(q->second->clone());
      }
      else
      {
        cl_task_t base = boost::static_pointer_cast<cl_task>(boost::shared_ptr<T>(new T));
        base->init(p->second.begin()->second->program);
        p->second.insert(std::make_pair(thread_id, base));
        return boost::dynamic_pointer_cast<T>(base->clone());
      }
    }
    else
    {
      cl_task_t base = boost::static_pointer_cast<cl_task>(boost::shared_ptr<T>(new T));
      base->init();
      vcl_map<int, cl_task_t> thread_map;
      thread_map.insert(std::make_pair(thread_id, base));
      tasks.insert(std::make_pair(task_name, thread_map));
      return boost::dynamic_pointer_cast<T>(base->clone());
    }
  }


private:

  cl_task_registry() { }
  static cl_task_registry *inst_;

  vcl_map<vcl_string, vcl_map<int, cl_task_t > > tasks;
};

#endif
