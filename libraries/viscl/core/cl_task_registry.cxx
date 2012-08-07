/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_task_registry.h"

namespace viscl
{

cl_task_registry *cl_task_registry::inst_ = 0;

cl_task_registry *cl_task_registry::inst()
{
  return inst_ ? inst_ : inst_ = new cl_task_registry;
}

}