#ifndef CL_PROGRAM_REGISTRY
#define CL_PROGRAM_REGISTRY

/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <map>

#include "header.h"
#include "config.h"

namespace viscl
{

class VISCL_EXPORT program_registry
{
public:

  static program_registry *inst();

  bool is_registered(const std::string &program_name);
  std::pair<cl_program_t, bool> get_program(const std::string &program_name);
  cl_program_t register_program(const std::string &program_name, const char *source);

private:

  program_registry() { }
  static program_registry *inst_;

  std::map<std::string, cl_program_t> prgms;
};

}

#endif
