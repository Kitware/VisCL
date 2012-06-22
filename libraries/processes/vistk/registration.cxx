/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "registration.h"

#include "vistk_homography_process.h"
#include "vistk_track_descr_match_process.h"

#include <vistk/pipeline/process_registry.h>

/**
 * \file porting/registration.cxx
 *
 * \brief Register processes for use.
 */

using namespace vistk;

void
register_processes()
{
  static process_registry::module_t const module_name = process_registry::module_t("viscl2vistk_processes");

  process_registry_t const registry = process_registry::self();

  if (registry->is_module_loaded(module_name))
  {
    return;
  }

  registry->register_process("viscl_track_descr_match", "Wrapper around viscl track_descr_match_process", create_process<vistk_track_descr_match_process>);
  registry->register_process("viscl_homography", "Wrapper around viscl homography_process", create_process<vistk_homography_process>);

  registry->mark_module_as_loaded(module_name);
}
