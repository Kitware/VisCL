#include <vcl_cstdlib.h>
#include <vcl_ctime.h>
#include <vcl_cstring.h>
#include <vcl_string.h>
#include <vcl_iostream.h>
#include <vcl_vector.h>
#include <vul/vul_arg.h>

#include <boost/chrono.hpp>

#include <CL/cl.hpp>

//******************************************************************************

struct ocl_info
{
  cl::Platform p;
  cl::Device d;
  cl::Context c;
  cl::CommandQueue q;

  ocl_info(unsigned pid, unsigned did)
  {
    vcl_vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    vcl_cout << "# Platforms: " << platforms.size() << vcl_endl;
    for(size_t i = 0; i < platforms.size(); ++i)
    {
      vcl_string name;
      platforms[i].getInfo(CL_PLATFORM_NAME, &name);
      vcl_cout << "Platform[" << i << "] : " << name << vcl_endl;
    }
    this->p = platforms[pid];

    vcl_cout << vcl_endl;

    vcl_vector<cl::Device> devices;
    this->p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    vcl_cout << "# Devices: " << devices.size() << vcl_endl;
    for(size_t i = 0; i < devices.size(); ++i)
    {
      vcl_string name;
      devices[i].getInfo(CL_DEVICE_NAME, &name);
      vcl_cout << "Device[" << i << "] : " << name << vcl_endl;
    }
    this->d = devices[did];

    vcl_vector<cl::Device> ctx_devices(1, this->d);
    this->c = cl::Context(ctx_devices);

    this->q = cl::CommandQueue(this->c, this->d);
  }
};

ocl_info *ocl;

//******************************************************************************

bool allocateHost(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  unsigned ni, bool use_images, bool pinned);

//******************************************************************************

bool freeHost(
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  bool use_images, bool pinned);

//******************************************************************************

bool allocateDevice(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  unsigned ni, bool use_images);

//******************************************************************************

bool freeDevice(
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  bool use_images);

//******************************************************************************

bool fillRand(vcl_vector<unsigned char*> &host_raw, size_t img_len);

//******************************************************************************

bool copyHostToDev(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  bool use_images, bool mapped,
  unsigned nj);

//******************************************************************************

int main(int argc, char **argv)
{
  vul_arg<unsigned> arg_clPlatform("-p", "OpenCL Platform ID", 0);
  vul_arg<unsigned> arg_clDevice("-d", "OpenCL Device ID", 0);
  vul_arg<unsigned> arg_iw("-iw", "Image width", 4096);
  vul_arg<unsigned> arg_ih("-ih", "Image height", 4096);
  vul_arg<unsigned> arg_ip("-ip", "Image planes", 1);
  vul_arg<unsigned> arg_ie("-ie", "Image element size (1: byte, 4: float)", 1);
  vul_arg<unsigned> arg_ni("-ni", "# of images", 1);
  vul_arg<unsigned> arg_nj("-nj", "# of iterations", 10);
  vul_arg<bool> arg_memImg("-i", "Use images instead of buffers", false);
  vul_arg<bool> arg_hostMem("-hp", "Use pinned host memory", false);
  vul_arg<bool> arg_devMem("-dm", "Use mapped device memory", false);

  vul_arg_parse(argc, argv);

  unsigned p = arg_clPlatform();
  unsigned d = arg_clDevice();
  unsigned iw = arg_iw();
  unsigned ih = arg_ih();
  unsigned ip = arg_ip();
  unsigned ie = arg_ie();
  unsigned ni = arg_ni();
  unsigned nj = arg_nj();
  bool use_images = arg_memImg();
  bool pinned = arg_hostMem();
  bool mapped = arg_devMem();
  bool ret;

  ocl = new ocl_info(p, d);

  vcl_vector<cl::Image2D*>   host_img;
  vcl_vector<cl::Buffer*>    host_buf;
  vcl_vector<unsigned char*> host_raw;
  vcl_vector<cl::Image2D*>   dev_img;
  vcl_vector<cl::Buffer*>    dev_buf;

  vcl_cout << "\n\nAlocating host memory..." << vcl_flush;
  ret = allocateHost(iw, ih, ip, ie, host_img, host_buf, host_raw,
    ni, use_images, pinned);
  vcl_cout << "done" << vcl_endl;

  vcl_cout << "Randomizing host memory..." << vcl_flush;
  ret = fillRand(host_raw, iw*ih*ip*ie);
  vcl_cout << "done" << vcl_endl;

  vcl_cout << "Alocating device memory..." << vcl_flush;
  ret = allocateDevice(iw, ih, ip, ie, dev_img, dev_buf, ni, use_images);
  vcl_cout << "done" << vcl_endl;

  vcl_cout << "Copying host to device..." << vcl_flush;
  ocl->q.finish();

  boost::chrono::system_clock::time_point t_start = boost::chrono::system_clock::now();
  ret = copyHostToDev(iw, ih, ip, ie,
    host_img, host_buf, host_raw,
    dev_img, dev_buf,
    use_images, mapped,
    nj);
  boost::chrono::system_clock::time_point t_stop = boost::chrono::system_clock::now();
  vcl_cout << "done" << vcl_endl;

  vcl_cout << "Freeing device memory..." << vcl_flush;
  ret = freeDevice(dev_img, dev_buf, use_images);
  vcl_cout << "done" << vcl_endl;

  vcl_cout << "Freeing host memory..." << vcl_flush;
  ret = freeHost(host_img, host_buf, host_raw, use_images, pinned);
  vcl_cout << "done" << vcl_endl;

  // Convert time struct to total seconds

  boost::chrono::duration<double> sec = t_stop - t_start;
  double t_seconds = sec.count();
  vcl_cout << "\n\nImage size: " << iw << 'x' << ih << 'x' << ip << " ("
           << ((iw*ih*ip*ie) / (1024.0*1024)) << "MB)" << vcl_endl;
  vcl_cout << ni*nj << " seperate transfers" << vcl_endl;
  vcl_cout << t_seconds << "s total" << vcl_endl;
  vcl_cout << (t_seconds/(ni*nj)) << "s per image" << vcl_endl;
  double img_size = iw*ih*ip*ie;
  double total_size = img_size*ni*nj;
  vcl_cout << (total_size/t_seconds)/(1024.0*1024.0) << " MB/s" << vcl_endl;

  delete ocl;

  return EXIT_FAILURE;
}

//******************************************************************************

bool allocateHost(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  unsigned ni, bool use_images, bool pinned)
{
  cl_channel_order co;
  switch(ip)
  {
    case 1: co = CL_INTENSITY; break;
    case 2: co = CL_RG; break;
    case 3: co = CL_RGB; break;
    case 4: co = CL_RGBA; break;
    default:
      vcl_cerr << "Unsupported number of planes" << vcl_endl;
      return false;
  }

  cl_channel_type ct;
  switch(ie)
  {
    case 1: ct = CL_UNORM_INT8; break;
    case 2: ct = CL_UNORM_INT16; break;
    case 4: ct = CL_FLOAT; break;
    default:
      vcl_cerr << "Unsupported image element size" << vcl_endl;
      return false;
  }

  cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
  cl::size_t<3> region; region[0] = iw; region[1] = ih; region[2] = 1;

  host_img.clear();  host_img.reserve(ni);
  host_buf.clear();  host_buf.reserve(ni);
  host_raw.clear();  host_raw.reserve(ni);

  size_t size = iw*ih*ip*ie;
  for(size_t i = 0; i < ni; ++i)
  {
    cl::Image2D *img = NULL;
    cl::Buffer  *buf = NULL;
    unsigned char *raw = NULL;
    if(pinned)
    {
      if(use_images)
      {
        img = new cl::Image2D(
          ocl->c, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
          cl::ImageFormat(co, ct), iw, ih);

        size_t row_pitch;
        raw = reinterpret_cast<unsigned char*>(
          ocl->q.enqueueMapImage(*img, CL_TRUE, CL_MAP_WRITE,
            origin, region, &row_pitch, NULL));
      }
      else
      {
        buf = new cl::Buffer(ocl->c,
          CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size);

        raw = reinterpret_cast<unsigned char*>(
          ocl->q.enqueueMapBuffer(*buf, CL_TRUE, CL_MAP_WRITE, 0, size));
      }
    }
    else
    {
      raw = new unsigned char[size];
    }
    host_img.push_back(img);
    host_buf.push_back(buf);
    host_raw.push_back(raw);
  }

  return true;
}

//******************************************************************************

bool freeHost(
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  bool use_images, bool pinned)
{
  size_t ni = host_raw.size();
  for(size_t i = 0; i < ni; ++i)
  {
    if(pinned)
    {
      if(use_images)
      {
        ocl->q.enqueueUnmapMemObject(*host_img[i], host_raw[i]);
      }
      else
      {
        ocl->q.enqueueUnmapMemObject(*host_buf[i], host_raw[i]);
      }
    }
    else
    {
      delete[] host_raw[i];
    }
  }
  ocl->q.finish();

  for(size_t i = 0; i < ni; ++i)
  {
    delete host_img[i];
    host_img[i] = NULL;

    delete host_buf[i];
    host_buf[i] = NULL;

    host_raw[i] = NULL;
  }

  host_img.clear();
  host_buf.clear();
  host_raw.clear();

  return false;
}

//******************************************************************************

bool allocateDevice(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  unsigned ni, bool use_images)
{
  cl_channel_order co;
  switch(ip)
  {
    case 1: co = CL_INTENSITY; break;
    case 2: co = CL_RG; break;
    case 3: co = CL_RGB; break;
    case 4: co = CL_RGBA; break;
    default:
      vcl_cerr << "Unsupported number of planes" << vcl_endl;
      return false;
  }

  cl_channel_type ct;
  switch(ie)
  {
    case 1: ct = CL_UNORM_INT8; break;
    case 2: ct = CL_UNORM_INT16; break;
    case 4: ct = CL_FLOAT; break;
    default:
      vcl_cerr << "Unsupported image element size" << vcl_endl;
      return false;
  }

  cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
  cl::size_t<3> region; region[0] = iw; region[1] = ih; region[2] = 1;

  dev_img.clear();  dev_img.reserve(ni);
  dev_buf.clear();  dev_buf.reserve(ni);

  size_t size = iw*ih*ip*ie;
  for(size_t i = 0; i < ni; ++i)
  {
    if(use_images)
    {
      cl::Image2D *img = new cl::Image2D(
        ocl->c, CL_MEM_READ_ONLY,
        cl::ImageFormat(co, ct), iw, ih);

        dev_img.push_back(img);
        dev_buf.push_back(NULL);
    }
    else
    {
      cl::Buffer *buf = new cl::Buffer(ocl->c,
        CL_MEM_READ_ONLY, size);

      dev_img.push_back(NULL);
      dev_buf.push_back(buf);
    }
  }
  ocl->q.finish();

  return true;
}

//******************************************************************************

bool freeDevice(
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  bool use_images)
{
  size_t ni = dev_img.size();
  for(size_t i = 0; i < ni; ++i)
  {
    if(use_images)
    {
      delete dev_img[i];
      dev_img[i] = NULL;
    }
    else
    {
      delete dev_buf[i];
      dev_buf[i] = NULL;
    }
  }
  ocl->q.finish();
  dev_img.clear();
  dev_buf.clear();
  return true;
}

//******************************************************************************

bool fillRand(vcl_vector<unsigned char*> &host_raw, size_t img_len)
{
  vcl_srand(vcl_time(NULL));

  size_t len_4 = img_len / sizeof(int);

  for(size_t j = 0; j < host_raw.size(); ++j)
  {
    int *ptr = reinterpret_cast<int*>(host_raw[j]);
    for(size_t i = 0; i < len_4; ++i)
    {
      ptr[i] = vcl_rand();
    }
  }
  return true;
}

//******************************************************************************

bool copyHostToDev(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  vcl_vector<cl::Image2D*>   &host_img,
  vcl_vector<cl::Buffer*>    &host_buf,
  vcl_vector<unsigned char*> &host_raw,
  vcl_vector<cl::Image2D*>   &dev_img,
  vcl_vector<cl::Buffer*>    &dev_buf,
  bool use_images, bool mapped,
  unsigned nj)
{
  cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
  cl::size_t<3> region; region[0] = iw; region[1] = ih; region[2] = 1;
  size_t len = iw*ih*ip*ie;
  size_t ni = host_img.size();
  for(size_t j = 0; j < nj; ++j)
  {
    for(size_t i = 0; i < ni; ++i)
    {
      if(mapped)
      {
        if(use_images)
        {
          size_t row_pitch;
          unsigned char *dev_raw = reinterpret_cast<unsigned char*>(
            ocl->q.enqueueMapImage(*dev_img[i], CL_TRUE, CL_MAP_WRITE,
              origin, region, &row_pitch, NULL));

          vcl_memcpy(dev_raw, host_raw[i], len);

          ocl->q.enqueueUnmapMemObject(*dev_img[i], dev_raw);
        }
        else
        {
          unsigned char *dev_raw = reinterpret_cast<unsigned char*>(
            ocl->q.enqueueMapBuffer(*dev_buf[i], CL_TRUE, CL_MAP_WRITE,
              0, len));

          vcl_memcpy(dev_raw, host_raw[i], len);

          ocl->q.enqueueUnmapMemObject(*dev_buf[i], dev_raw);
        }
      }
      else
      {
        if(use_images)
        {
          ocl->q.enqueueWriteImage(*dev_img[i], CL_TRUE, origin, region,
            0, 0, host_raw[i]);
        }
        else
        {
          ocl->q.enqueueWriteBuffer(*dev_buf[i], CL_TRUE, 0, len, host_raw[i]);
        }
      }
    }
  }
  ocl->q.finish();
  return true;
}

//******************************************************************************

