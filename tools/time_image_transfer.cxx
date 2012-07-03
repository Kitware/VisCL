#include <vcl_cstdlib.h>
#include <vcl_ctime.h>
#include <vcl_iostream.h>
#include <vcl_vector.h>
#include <vul/vul_arg.h>

#include <cl_manager.h>

//*********************************************************************

double fill_rand(vcl_vector<unsigned char *> &data, size_t ilen);

double allocate_gpu(unsigned w, unsigned h, unsigned n,
  vcl_vector<cl::Image2D *> &data);

double cleanup_gpu(vcl_vector<cl::Image2D *> &data);

double allocate_host(unsigned w, unsigned h, unsigned n,
  vcl_vector<unsigned char *> &data);

double cleanup_host(vcl_vector<unsigned char *> &raw_data);

double allocate_pinned_host(unsigned w,  unsigned h, unsigned n,
  vcl_vector<cl::Image2D*> &host_data, vcl_vector<unsigned char *> &raw_data);

double cleanup_pinned_host(
  vcl_vector<cl::Image2D*> &host_data, vcl_vector<unsigned char *> &raw_data);

double copy_to_gpu(
  const vcl_vector<unsigned char *> &raw_data,
  vcl_vector<cl::Image2D*> gpu_data,
  unsigned w, unsigned h, unsigned nj);

//*********************************************************************

struct ocl_objs
{
  const cl::Device &dev;
  const cl::Context &ctx;
  cl::CommandQueue q;

  ocl_objs(cl_manager *m)
  : dev(m->get_device()), ctx(m->get_context()), q(ctx, dev)
  { }
};
ocl_objs *g_ocl;

//*********************************************************************

int main(int argc, char **argv)
{
  vul_arg<unsigned> arg_w("-w", "Width of image to transfer.", 4096);
  vul_arg<unsigned> arg_h("-h", "Height of image to transfer.", 4096);
  vul_arg<unsigned> arg_p("-p", "Number of image planes.", 1);
  vul_arg<unsigned> arg_ni("-i", "Number of images to cycle through.", 10);
  vul_arg<unsigned> arg_nj("-j", "Number of outerations.", 10);
  vul_arg<bool>     arg_pinned("--pinned", "Use pinned memory", false);
  
  vul_arg_parse(argc, argv);

  unsigned w = arg_w();
  unsigned h = arg_h();
  unsigned p = arg_p();
  unsigned ni = arg_ni();
  unsigned nj = arg_nj();
  bool use_pinned = arg_pinned();

  unsigned img_len = w*h*p;

  double t_elapsed;
  double num_transfers;
  double size_per_transfer;
  double total_size_transfer;
  try
  {
    g_ocl = new ocl_objs(cl_manager::inst());

    vcl_vector<cl::Image2D*> gpu_data;
    vcl_vector<cl::Image2D*> host_data;
    vcl_vector<unsigned char *> raw_data;

    allocate_gpu(w, h, ni, gpu_data);

    if(use_pinned)
      allocate_pinned_host(w, h, ni, host_data, raw_data);
    else
      allocate_host(w, h, ni, raw_data);

    fill_rand(raw_data, img_len);

    t_elapsed = copy_to_gpu(raw_data, gpu_data, w, h, nj);

    if(use_pinned)
      cleanup_pinned_host(host_data, raw_data);
    else
      cleanup_host(raw_data);

    num_transfers = ni*nj;
    size_per_transfer = w*h;
    total_size_transfer = num_transfers * size_per_transfer;
    vcl_cout << "Time             : " << t_elapsed << " seconds\n"
             << "Num transfers    : " << num_transfers << "\n"
             << "Size per transfer: " << (size_per_transfer/(1024*1024)) << "MB\n"
             << "Time per transfer: " << ((t_elapsed / num_transfers)/1e-3) << " ms\n"
             << "Speed            : " << ((total_size_transfer/(1024*1024)) / t_elapsed) << " MB/s" << vcl_endl;
    vcl_cout << "\n\n" << vcl_endl;


   cleanup_gpu(gpu_data);
  }
  catch(const cl::Error &e)
  {
    vcl_cerr << "OpenCL Error: " << e.what() << ": " << e.err() << vcl_endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//*********************************************************************

double fill_rand(vcl_vector<unsigned char *> &data, size_t ilen)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Filling with random data..." << vcl_flush;
  vcl_srand(vcl_time(NULL));
  unsigned ni_int = ilen / 4;
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < data.size(); ++i)
  {
    int *datai = reinterpret_cast<int*>(data[i]);
    for(size_t j = 0; j < ni_int; ++j)
    {
      datai[j] = vcl_rand();
    }
  }
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double allocate_gpu(unsigned w, unsigned h, unsigned n,
  vcl_vector<cl::Image2D*> &gpu_data)

{
  timespec ts_start, ts_stop;
  vcl_cout << "Allocating GPU memory for " << n << " images of "
           << ((w*h) >> 20) << "MB each..." << vcl_flush;

  gpu_data.clear();
  gpu_data.reserve(n);
  for(size_t i = 0; i < n; ++i)
  {
    int err;
    cl::Image2D *img = new cl::Image2D(
      g_ocl->ctx, 0,
      cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8),
      w, h, 0, NULL, &err);
    if(err != CL_SUCCESS)
    {
      vcl_cout << "Error allocating GPU image " << i << vcl_endl;
      return -1.0;
    };
    gpu_data.push_back(img);
  }
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double cleanup_gpu(vcl_vector<cl::Image2D *> &gpu_data)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Cleaning up GPU memory..." << vcl_flush;
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < gpu_data.size(); ++i)
  {
    delete gpu_data[i];
  }
  gpu_data.clear();
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double allocate_host(unsigned w, unsigned h, unsigned n,
  vcl_vector<unsigned char *> &raw_data)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Allocating host memory for " << n << " images of "
           << ((w*h) >> 20) << "MB each..." << vcl_flush;
  raw_data.clear();
  raw_data.reserve(n);
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < n; ++i)
  {
    raw_data.push_back(new unsigned char[w*h]);
  }
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double cleanup_host(vcl_vector<unsigned char *> &raw_data)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Cleaning up host memory..." << vcl_flush;
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < raw_data.size(); ++i)
  {
    delete[] raw_data[i];
  }
  raw_data.clear();
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}  

//*********************************************************************

double allocate_pinned_host(unsigned w,  unsigned h, unsigned n,
  vcl_vector<cl::Image2D*> &host_data, vcl_vector<unsigned char *> &raw_data)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Allocating paged memory for " << n << " images of "
           << ((w*h) >> 20) << "MB each..." << vcl_flush;

  host_data.clear();
  host_data.reserve(n);
  raw_data.clear();
  raw_data.reserve(n);
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < n; ++i)
  {
    int err;

    cl::Image2D *img = new cl::Image2D(
      g_ocl->ctx, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
      cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8),
      w, h, 0, NULL, &err);
    if(err != CL_SUCCESS)
    {
      vcl_cout << "Error allocating image " << i << vcl_endl;
      return -1.0;
    };
    host_data.push_back(img);                                   
      
    size_t row_pitch;
    cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
    cl::size_t<3> region; region[0] = w; region[1] = h; region[2] = 1;

    unsigned char *img_ptr = reinterpret_cast<unsigned char*>(
      g_ocl->q.enqueueMapImage(*img, CL_TRUE, CL_MAP_WRITE,
        origin, region, &row_pitch, NULL, NULL, NULL, &err));
    if(err != CL_SUCCESS)
    {
      vcl_cout << "Error mapping image " << i << vcl_endl;
      return -1.0;
    };
    raw_data.push_back(img_ptr);
  }  
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double cleanup_pinned_host(
  vcl_vector<cl::Image2D*> &host_data, vcl_vector<unsigned char *> &raw_data)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Cleaning up paged host memory..." << vcl_flush;

  size_t n = host_data.size();

  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t i = 0; i < n; ++i)
  {
    int err = g_ocl->q.enqueueUnmapMemObject(
      *host_data[i], raw_data[i]);
    
    if(err != CL_SUCCESS)
    {
      vcl_cout << "Error unmapping image " << i << vcl_endl;
      return -1.0;
    };
    delete host_data[i];
  }
  host_data.clear();
  raw_data.clear();
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

double copy_to_gpu(
  const vcl_vector<unsigned char *> &raw_data,
  vcl_vector<cl::Image2D*> gpu_data,
  unsigned w, unsigned h, unsigned nj)
{
  timespec ts_start, ts_stop;
  vcl_cout << "Transfering image data to GPU..." << vcl_flush;

  size_t ni = raw_data.size();

  cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
  cl::size_t<3> region; region[0] = w; region[1] = h; region[2] = 1;
  clock_gettime(CLOCK_REALTIME, &ts_start);
  for(size_t j = 0; j < nj; ++j)
  {
    for(size_t i = 0; i < ni; ++i)
    {
      unsigned char *raw_img = raw_data[i];
      cl::Image2D *gpu_img = gpu_data[i];

      g_ocl->q.enqueueWriteImage(*gpu_img, CL_TRUE, origin, region, w, 0, raw_img);
    }
  }
  clock_gettime(CLOCK_REALTIME, &ts_stop);
  vcl_cout << "Done" << vcl_endl;

  for(size_t i = 0; i < ni; ++i)
  {
    delete gpu_data[i];
  }

  // Convert time struct to total seconds
  double t_start = ts_start.tv_sec + 1e-9*ts_start.tv_nsec;
  double t_stop = ts_stop.tv_sec + 1e-9*ts_stop.tv_nsec;
  return t_stop - t_start;
}

//*********************************************************************

