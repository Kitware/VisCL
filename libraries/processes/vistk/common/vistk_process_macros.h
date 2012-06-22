/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_MACROS_H
#define VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_MACROS_H

#define DEFINE_STEP_VARIABLES \
  bool complete = false;      \
  bool skip = false;          \
  (void)skip;
#define IF_FAILED() \
  if (complete)
#define IF_SKIPPED() \
  if (skip)

#define STEP_PROCESS(proc)                                      \
  do                                                            \
  {                                                             \
    if (!complete)                                              \
    {                                                           \
      vidtk::process::step_status const status = proc->step2(); \
                                                                \
      switch (status)                                           \
      {                                                         \
        case vidtk::process::FAILURE:                           \
          complete = true;                                      \
          break;                                                \
        case vidtk::process::SKIP:                              \
          skip = true;                                          \
          break;                                                \
        case vidtk::process::SUCCESS:                           \
        default:                                                \
          break;                                                \
      }                                                         \
    }                                                           \
  } while (false)

#define TINPUT_BRIDGE(proc, trans, port, type) \
  proc->set_##port(trans(grab_from_port_as<type>(#port)))
#define TOUTPUT_BRIDGE(proc, trans, port, type) \
  push_to_port_as<type>(#port, trans(proc->port()))

#define INPUT_BRIDGE(proc, port, type) \
  TINPUT_BRIDGE(proc, , port, type)
#define OUTPUT_BRIDGE(proc, port, type) \
  TOUTPUT_BRIDGE(proc, , port, type)

#define TINPUT_CHECK(proc, trans, port, ptype)                     \
  do                                                               \
  {                                                                \
    vistk::datum_t const dat_##port = grab_datum_from_port(#port); \
                                                                   \
    switch (dat_##port->type())                                    \
    {                                                              \
      case vistk::datum::data:                                     \
        proc->set_##port(trans(dat_##port->get_datum<ptype>()));   \
        break;                                                     \
      case vistk::datum::complete:                                 \
        complete = true;                                           \
        break;                                                     \
      case vistk::datum::empty:                                    \
      case vistk::datum::error:                                    \
      case vistk::datum::flush:                                    \
      default:                                                     \
        break;                                                     \
    }                                                              \
  } while (false)
#define INPUT_CHECK(proc, port, type) \
  TINPUT_CHECK(proc, , port, type)

#define OUTPUT_COMPLETE(proc, port) \
  push_datum_to_port(#port, vistk::datum::complete_datum())
#define OUTPUT_EMPTY(proc, port) \
  push_datum_to_port(#port, vistk::datum::empty_datum())

#endif // VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_MACROS_H
