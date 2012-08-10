#ifndef CL_TASK_REGISTRY
#define CL_TASK_REGISTRY

/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <viscl/core/task.h>
#include <map>
#include <boost/make_shared.hpp>


//Cloning a task does not deep copy its kernel, therefore if a task is used in multiple threads
//there will be a race-condition for the kernel args.  Thus deep_clone should be used when
//using a kernel in a different thread

#define NEW_VISCL_TASK(T) viscl::task_registry::inst()->new_task<T>(std::string(#T), 0)
#define NEW_THREAD_SAFE_VISCL_TASK(T, thread_id) viscl::task_registry::inst()->new_task<T>(std::string(#T), (thread_id));


namespace viscl
{

class task_registry
{
public:

  static task_registry *inst();

  template<class T>
  boost::shared_ptr<T> new_task(const std::string &task_name, unsigned int thread_id)
  {
    std::map<std::string, std::map<int, task_t> >::iterator p = tasks.find(task_name);
    if (p != tasks.end())
    {
      std::map<int, task_t>::const_iterator q = p->second.find(thread_id);
      if (q != p->second.end())
      {
        return boost::dynamic_pointer_cast<T>(q->second->clone());
      }
      else
      {
        task_t base = boost::static_pointer_cast<task>(boost::shared_ptr<T>(new T));
        base->init(p->second.begin()->second->program);
        p->second.insert(std::make_pair(thread_id, base));
        return boost::dynamic_pointer_cast<T>(base->clone());
      }
    }
    else
    {
      task_t base = boost::static_pointer_cast<task>(boost::shared_ptr<T>(new T));
      base->init();
      std::map<int, task_t> thread_map;
      thread_map.insert(std::make_pair(thread_id, base));
      tasks.insert(std::make_pair(task_name, thread_map));
      return boost::dynamic_pointer_cast<T>(base->clone());
    }
  }


private:

  task_registry() { }
  static task_registry *inst_;

  std::map<std::string, std::map<int, task_t > > tasks;
};

}

#endif
