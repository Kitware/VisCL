/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_PROCESSES_PORTING_CONFIG_H_
#define VISCL_PROCESSES_PORTING_CONFIG_H_

#include <vistk/config.h>

/**
 * \file porting-config.h
 *
 * \brief Defines for symbol visibility in porting processes.
 */

#ifndef VISCL_PROCESSES_PORTING_EXPORT
#ifdef MAKE_VISCL_PROCESSES_PORTING_LIB
/// Export the symbol if building the library.
#define VISCL_PROCESSES_PORTING_EXPORT VISTK_EXPORT
#else
/// Import the symbol if including the library.
#define VISCL_PROCESSES_PORTING_EXPORT VISTK_IMPORT
#endif // MAKE_VISCL_PROCESSES_PORTING_LIB
/// Hide the symbol from the library interface.
#define VISCL_PROCESSES_PORTING_NO_EXPORT VISTK_NO_EXPORT
#endif // VISCL_PROCESSES_PORTING_EXPORT

#ifndef VISCL_PROCESSES_PORTING_EXPORT_DEPRECATED
/// Mark as deprecated.
#define VISCL_PROCESSES_PORTING_EXPORT_DEPRECATED VISTK_DEPRECATED VISCL_PROCESSES_PORTING_EXPORT
#endif // VISCL_PROCESSES_PORTING_EXPORT_DEPRECATED

#endif // VISCL_PROCESSES_PORTING_CONFIG_H_
