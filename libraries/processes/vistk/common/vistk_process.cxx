/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vistk_process.h"

#include <process_framework/process.h>
#include <utilities/config_block.h>

#include <vistk/pipeline/config.h>
#include <vistk/pipeline/process.h>
#include <vistk/pipeline/process_exception.h>

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>

#include <map>

namespace viscl
{

vistk::process::property_t const vistk_process::constraint_vidtk = "_vidtk";

vistk_process
::vistk_process(vistk::config_t const& conf)
  : vistk::process(conf)
{
}

vistk::process::properties_t
vistk_process
::_properties() const
{
  properties_t base_properties = vistk::process::_properties();

  base_properties.insert(constraint_vidtk);

  return base_properties;
}

void
vistk_process
::declare_vistk_configuration(vidtk_process_t vidtk_process)
{
  typedef std::map<std::string, vidtk::config_block_value> config_value_map_t;

  vidtk::config_block const block = vidtk_process->params();
  config_value_map_t const values = block.enumerate_values();

  BOOST_FOREACH (config_value_map_t::value_type const& config_value, values)
  {
    vistk::config::key_t const& key = config_value.first;
    vidtk::config_block_value const& block_value = config_value.second;

    vistk::config::description_t const& desc = block_value.description_;
    vistk::config::value_t def;

    if (block_value.has_default_value())
    {
      def = block_value.default_;
    }

    declare_configuration_key(key, boost::make_shared<conf_info>(
      def,
      desc));

    m_keys.push_back(key);
  }
}

void
vistk_process
::configure_vidtk_process(vidtk_process_t vidtk_process)
{
  vidtk::config_block block = vidtk_process->params();

  BOOST_FOREACH (vistk::config::key_t const& key, m_keys)
  {
    vistk::config::value_t const value = config_value<std::string>(key);

    block.set(key, value);
  }

  if (!vidtk_process->set_params(block))
  {
    static std::string const reason = "Failed to configure the vidtk process";

    throw vistk::invalid_configuration_exception(name(), reason);
  }

  if (!vidtk_process->initialize())
  {
    static std::string const reason = "Failed to initialize the vidtk process";

    throw vistk::invalid_configuration_exception(name(), reason);
  }
}

}
