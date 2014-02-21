/*ckwg +29
 * Copyright 2012 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
