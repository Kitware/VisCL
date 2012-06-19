#include "track_descr_match.h"
#include "cl_manager.h"

#include "cl_task_registry.h"

#include <boost/make_shared.hpp>

extern const char* track_descr_match_source;

track_descr_match::track_descr_match() : cl_task(track_descr_match_source)
{
  track_k = make_kernel("track");
}

cl_task_t track_descr_match::clone()
{
  track_descr_match_t clone_ = boost::make_shared<track_descr_match>(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//Copy constructor for cloning, does not clone tracks
track_descr_match::track_descr_match(const track_descr_match &t)
{
  this->program = t.program;
  this->track_k = t.track_k;
}

template<class T>
void track_descr_match::first_frame(const vil_image_view<T> &img)
{  
  int max_kpts = 5000;
  float thresh = 0.007f, sigma = 2.0f;
  
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  gs = NEW_VISCL_TASK(gaussian_smooth);
  cl_image smoothed = gs->smooth(img_cl, sigma);
  img_cl.del();

  hes = NEW_VISCL_TASK(hessian);
  cl_buffer numkpts_b;
  cl_image kptmap_first;
  hes->detect(smoothed, kptmap_first, kpts1, numkpts_b, max_kpts, thresh, sigma);
  numkpts1 = hes->num_kpts(numkpts_b);
  vcl_cout << numkpts1 << "\n";

  brf = NEW_VISCL_TASK(brief<10>);
  brf->compute_descriptors(smoothed, kpts1, numkpts1, descriptors1);
}

template<class T>
void track_descr_match::track(vil_image_view<T> &img)
{  
  int max_kpts = 5000;
  float thresh = 0.007f, sigma = 2.0f;
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_image smoothed = gs->smooth(img_cl, sigma);
  img_cl.del(); 
  
  cl_buffer kpts2, numkpts2_b;
  cl_image kptmap;
  hes->detect(smoothed, kptmap, kpts2, numkpts2_b, max_kpts, thresh, sigma);
  int numkpts2 = hes->num_kpts(numkpts2_b);
  vcl_cout << numkpts2 << "\n";

  cl_buffer descriptors2;
  brf->compute_descriptors(smoothed, kpts2, numkpts2, descriptors2);

  cl_buffer tracks_b = cl_manager::inst()->create_buffer<int>(CL_MEM_WRITE_ONLY, numkpts1);

  const int window = 250; //window is radius / 2
  track_k->setArg(0, *kpts1().get());
  track_k->setArg(1, *kptmap().get());
  track_k->setArg(2, *descriptors1().get());
  track_k->setArg(3, *descriptors2().get());
  track_k->setArg(4, *tracks_b().get());
  track_k->setArg(5, window);

  cl::NDRange global(numkpts1);
  queue->enqueueNDRangeKernel(*track_k.get(), cl::NullRange, global, cl::NullRange);

  int *indices = new int[numkpts1];
  queue->enqueueReadBuffer(*tracks_b(), CL_TRUE, 0, tracks_b.mem_size(), indices);

  vcl_vector<cl_int2> kpts1_v(numkpts1), kpts2_v(numkpts2);
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts1, &kpts1_v[0]);
  queue->enqueueReadBuffer(*kpts2().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts2, &kpts2_v[0]);

  tracks.resize(numkpts1);
  for (int i = 0; i < numkpts1; i++)
  {
    tracks[i].pt_last = vnl_int_2(kpts1_v[i].s[0], kpts1_v[i].s[1]);
    tracks[i].pt_new = vnl_int_2(kpts2_v[indices[i]].s[0], kpts2_v[indices[i]].s[1]);
  }

  //vcl_ofstream outfile("tracks.txt");
  //for (int i = 0; i < numkpts1; i++)
  //{
  //  if (indices[i] != -1)
  //    outfile << kpts1_v[i].s[0] << " " << kpts1_v[i].s[1] << " " << kpts2_v[indices[i]].s[0] << " " << kpts2_v[indices[i]].s[1] << "\n";
  //}
  //outfile.close();

  kpts1 = kpts2;
  numkpts1 = numkpts2;
  descriptors1 = descriptors2;
  delete [] indices;
}

template void track_descr_match::first_frame(const vil_image_view<vxl_byte> &img);
template void track_descr_match::track(vil_image_view<vxl_byte> &img);

