/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef HOMOGRAPHY_PROCESS_H_
#define HOMOGRAPHY_PROCESS_H_

#include "track_descr_match.h"

#include <vcl_vector.h>
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <rrel/rrel_ran_sam_search.h>

#include <process_framework/process.h>
#include <process_framework/pipeline_aid.h>
#include <utilities/homography.h>
#include <utilities/config_block.h>

class homography_process : public vidtk::process
{
public:

  typedef homography_process self_type;

  homography_process( vcl_string const& name );
  ~homography_process();

  vidtk::config_block params() const;
  bool set_params( vidtk::config_block const& );
  bool initialize();
  bool reset();
  bool step();

  void set_tracks( vcl_vector< tdm_track > const& trks );
  VIDTK_INPUT_PORT( set_tracks, vcl_vector< tdm_track > const& );

  void set_timestamp( vidtk::timestamp const & ts );
  VIDTK_OPTIONAL_INPUT_PORT( set_timestamp, vidtk::timestamp const & );

  vgl_h_matrix_2d<double> const& img_to_img0_homography() const;
  VIDTK_OUTPUT_PORT( vgl_h_matrix_2d<double> const&, img_to_img0_homography );

  vgl_h_matrix_2d<double> const& img0_to_img_homography() const;
  VIDTK_OUTPUT_PORT( vgl_h_matrix_2d<double> const&, img0_to_img_homography );

  vidtk::image_to_image_homography const& img_to_img0_i2i_homography() const;
  VIDTK_OUTPUT_PORT( vidtk::image_to_image_homography const&, img_to_img0_i2i_homography );

  vidtk::image_to_image_homography const& img0_to_img_i2i_homography() const;
  VIDTK_OUTPUT_PORT( vidtk::image_to_image_homography const&, img0_to_img_i2i_homography );

private:

  void lmq_refine_homography(vidtk::image_to_image_homography &H);

  vcl_vector<track_descr_match::pt_track> tracks_;
  vcl_vector<bool> good_flags_;

  double good_thresh_sqr_;
  bool initialized_;
  bool lmq_refinement_;

  vidtk::config_block config_;

  vidtk::timestamp const * current_ts_;
  vidtk::timestamp reference_ts_;
  
  vidtk::image_to_image_homography img_to_img0;
  vidtk::image_to_image_homography img0_to_img;

  rrel_ran_sam_search * ransam_;
};

#endif
