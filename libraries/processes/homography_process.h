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



class homography_process : vidtk::process
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

  void set_new_tracks( vcl_vector< track_descr_match::pt_track > const& trks );
  VIDTK_INPUT_PORT( set_new_tracks, vcl_vector< track_descr_match::pt_track > const& );

  void set_timestamp( vidtk::timestamp const & ts );
  VIDTK_OPTIONAL_INPUT_PORT( set_timestamp, vidtk::timestamp const & );

  vgl_h_matrix_2d<double> const& source_to_ref_homography() const;
  VIDTK_OUTPUT_PORT( vgl_h_matrix_2d<double> const&, source_to_ref_homography );

  vgl_h_matrix_2d<double> const& ref_to_source_homography() const;
  VIDTK_OUTPUT_PORT( vgl_h_matrix_2d<double> const&, ref_to_source_homography );

  vidtk::image_to_image_homography const& source_to_ref_homography_image() const;
  VIDTK_OUTPUT_PORT( vidtk::image_to_image_homography const&, source_to_ref_homography_image );

  vidtk::image_to_image_homography const& ref_to_source_homography_image() const;
  VIDTK_OUTPUT_PORT( vidtk::image_to_image_homography const&, ref_to_source_homography_image );



private:

  void lmq_refine_homography();
  void set_img0_locations();
  void set_good_flags();

  struct track_meta_data
  {
    track_meta_data() : good(true), have_img0(false) {}
    bool good;
    bool have_img0;
    vgl_homg_point_2d<double> img0_loc;
  };

  vcl_vector<track_descr_match::pt_track> tracks_;
  vcl_vector<track_meta_data> tracks_meta_data_;

  double good_thresh_sqr_;
  bool initialized_;

  vidtk::config_block config_;

  vidtk::timestamp const * current_ts_;
  vidtk::timestamp reference_ts_;
  
  vidtk::image_to_image_homography img_to_img0;
  vidtk::image_to_image_homography img0_to_img;

  rrel_ran_sam_search * ransam_;
};

#endif
