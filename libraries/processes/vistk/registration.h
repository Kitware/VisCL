/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_PROCESSES_PORTING_REGISTRATION_H
#define VISCL_PROCESSES_PORTING_REGISTRATION_H

#include "porting-config.h"

/**
 * \file porting/registration.h
 *
 * \brief Register processes for use.
 */

extern "C"
{

/**
 * \brief Register processes.
 */
void VISCL_PROCESSES_PORTING_EXPORT register_processes();

}

#endif // VISCL_PROCESSES_PORTING_REGISTRATION_H
