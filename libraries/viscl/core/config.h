/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_CONFIG_H_
#define VISCL_CONFIG_H_


// Visibility macros
#if defined(_WIN32) || defined(_WIN64)
# define VISCL_EXPORT __declspec(dllexport)
# define VISCL_IMPORT __declspec(dllimport)
# define VISCL_NO_EXPORT
#elif defined(VISCL_HAVE_GCC_VISIBILITY)
# define VISCL_EXPORT __attribute__((__visibility__("default")))
# define VISCL_IMPORT __attribute__((__visibility__("default")))
# define VISCL_NO_EXPORT __attribute__((__visibility__("hidden")))
#else
# define VISCL_EXPORT
# define VISCL_IMPORT
# define VISCL_NO_EXPORT
#endif


#endif // VISCL_CONFIG_H_
