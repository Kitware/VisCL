/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "task_registry.h"

namespace viscl
{

task_registry *task_registry::inst_ = 0;

task_registry *task_registry::inst()
{
  return inst_ ? inst_ : inst_ = new task_registry;
}

}