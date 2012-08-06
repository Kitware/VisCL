/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef TRACK_DESCR_MATCH_PROCESS_H_
#define TRACK_DESCR_MATCH_PROCESS_H_

#include <utilities/timestamp.h>
#include <process_framework/process.h>
#include <process_framework/pipeline_aid.h>
#include <kwklt/klt_track.h>

#include <vil/vil_image_view.h>

#include <viscl/tasks/track_descr_match.h>

/// \brief Track features across images using descriptor matches.
///
/// \note This tracker will only work for contiguous greyscale
/// images.
class track_descr_match_process
  : public vidtk::process
{
public:
  typedef track_descr_match_process self_type;

  track_descr_match_process( vcl_string const& name );
  ~track_descr_match_process();

  vidtk::config_block params() const;
  bool set_params( vidtk::config_block const& );
  bool initialize();
  bool reinitialize();
  bool step();
  bool reset();

  /// Set the next image to process.
  void set_image( vil_image_view< vxl_byte > const& img );
  VIDTK_INPUT_PORT( set_image, vil_image_view< vxl_byte > const& );

  /// Set of tracks that are actively being tracked.
  vcl_vector< vidtk::klt_track_ptr > const& active_tracks() const;
  VIDTK_OUTPUT_PORT( vcl_vector< vidtk::klt_track_ptr > const&, active_tracks );

  /// Set of new tracks that were created at the last step.
  vcl_vector<vidtk::klt_track_ptr> const& created_tracks() const;
  VIDTK_OUTPUT_PORT(vcl_vector<vidtk::klt_track_ptr> const&, created_tracks);

protected:

  // ----- Parameters
  vidtk::config_block config_;

  /// Maximum displacement allowed for a feature.
  unsigned int search_range_;

  /// The current frame.
  vil_image_view<vxl_byte> cur_img_;

  track_descr_match_t tracker_;

  /// Set of tracks corresponding to still active features
  vcl_vector< vidtk::klt_track_ptr > *tracks_last_, *tracks_cur_;
  vcl_vector< vidtk::klt_track_ptr > active_tracks_;
  vcl_vector< vidtk::klt_track_ptr > new_tracks_;

  // If the process is retained across two calls on intialize(), then
  // we only want to "initialize" only the first time.
  bool initialized_;
  bool first_frame_set_;

  int num_feat_;
};


#endif
