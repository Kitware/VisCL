/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "program_registry.h"
#include "manager.h"

namespace viscl
{

program_registry *program_registry::inst_ = 0;

//*****************************************************************************

program_registry *program_registry::inst()
{
  return inst_ ? inst_ : inst_ = new program_registry;
}

//*****************************************************************************

bool program_registry::is_registered(const std::string &program_name)
{
  return prgms.find(program_name) != prgms.end();
}

//*****************************************************************************

std::pair<cl_program_t, bool> program_registry::get_program(const std::string &program_name)
{
  std::map<std::string, cl_program_t>::iterator p = prgms.find(program_name);
  if (p != prgms.end())
    return std::make_pair(p->second, true);
  return std::make_pair(boost::shared_ptr<cl::Program>(), false);
}

//*****************************************************************************

cl_program_t program_registry::register_program(const std::string &program_name, const char *source)
{
  std::map<std::string, cl_program_t>::iterator p = prgms.find(program_name);
  if (p != prgms.end())
  {
    return p->second;
  }
  else
  {
    cl_program_t program = manager::inst()->build_source(source);
    prgms.insert(std::make_pair(program_name, program));
    return program;
  }
}

//*****************************************************************************

}
