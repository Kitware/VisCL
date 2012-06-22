/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vistk_homography_process.h"
#include "common/vistk_process_macros.h"
#include "conversion/timestamp.h"
#include "conversion/homography.h"

#include <boost/make_shared.hpp>

//*****************************************************************************

vistk_homography_process::vistk_homography_process(vistk::config_t const& conf) 
  : vistk_process(conf),
    w_proc(new viscl_proc_type(name()))
{
  this->declare_vistk_configuration(w_proc.get());

  port_flags_t required;
  port_flags_t const none;
  port_flags_t required_nodep;

  required.insert(flag_required);

  required_nodep.insert(flag_required);
  required_nodep.insert(flag_input_nodep);

  this->declare_input_port("tracks", boost::make_shared<port_info>("tracks/descr_match", required, "input tracks"));
  this->declare_input_port("timestamp", boost::make_shared<port_info>("timestamp", none, "timestamp"));
  
  this->declare_output_port("img_to_img0_homography", boost::make_shared<port_info>("matrix/vgl/2d/double", none, "src to ref homography matrix"));
  this->declare_output_port("img0_to_img_homography", boost::make_shared<port_info>("matrix/vgl/2d/double", none, "ref to src homography matrix"));
  this->declare_output_port("img_to_img0_i2i_homography", boost::make_shared<port_info>("homography/image:src/image:ref", none, "src to ref homography"));
  this->declare_output_port("img0_to_img_i2i_homography", boost::make_shared<port_info>("homography/image:ref/image:src", none, "ref to src homography"));

  this->declare_input_port("reset", boost::make_shared<port_info>("bool", required_nodep, "Whether to reset the process or not."));
}

//*****************************************************************************

void vistk_homography_process::_configure()
{
  configure_vidtk_process(w_proc.get());
  vistk_process::_configure();
}

//*****************************************************************************

void vistk_homography_process::_init()
{
  first = true;
  has_timestamp = !!input_port_edge("timestamp");
  vistk_process::_init();
}

//*****************************************************************************

void vistk_homography_process::_step()
{
  DEFINE_STEP_VARIABLES

  if (!first)
  {
    bool const reset = grab_from_port_as<bool>("reset");

    if (reset)
    {
      w_proc->reset();
    }
  }

  first = false;

  timestamp_conversion const ts_conv;

  INPUT_BRIDGE(w_proc, tracks, vcl_vector<tdm_track>);

  if (has_timestamp)
  {
    TINPUT_BRIDGE(w_proc, ts_conv, timestamp, vistk::timestamp);
  }

  STEP_PROCESS(w_proc);

  IF_FAILED()
  {
    OUTPUT_COMPLETE(w_proc, img_to_img0_homography);
    OUTPUT_COMPLETE(w_proc, img0_to_img_homography);
    OUTPUT_COMPLETE(w_proc, img_to_img0_i2i_homography);
    OUTPUT_COMPLETE(w_proc, img0_to_img_i2i_homography);
    mark_process_as_complete();
  }
  else 
  {
    image_to_image_homography_conversion const h_conv;
    OUTPUT_BRIDGE(w_proc, img_to_img0_homography, vgl_h_matrix_2d<double>);
    OUTPUT_BRIDGE(w_proc, img0_to_img_homography, vgl_h_matrix_2d<double>);
    TOUTPUT_BRIDGE(w_proc, h_conv, img_to_img0_i2i_homography, vistk::image_to_image_homography);
    TOUTPUT_BRIDGE(w_proc, h_conv, img0_to_img_i2i_homography, vistk::image_to_image_homography);
  }

  vistk_process::_step();
}

//*****************************************************************************

