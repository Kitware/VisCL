#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
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
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    std::cout << "# Platforms: " << platforms.size() << std::endl;
    for(size_t i = 0; i < platforms.size(); ++i)
    {
      std::string name;
      platforms[i].getInfo(CL_PLATFORM_NAME, &name);
      std::cout << "Platform[" << i << "] : " << name << std::endl;
    }
    this->p = platforms[pid];

    std::cout << std::endl;

    std::vector<cl::Device> devices;
    this->p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    std::cout << "# Devices: " << devices.size() << std::endl;
    for(size_t i = 0; i < devices.size(); ++i)
    {
      std::string name;
      devices[i].getInfo(CL_DEVICE_NAME, &name);
      std::cout << "Device[" << i << "] : " << name << std::endl;
    }
    this->d = devices[did];

    std::vector<cl::Device> ctx_devices(1, this->d);
    this->c = cl::Context(ctx_devices);

    this->q = cl::CommandQueue(this->c, this->d);
  }
};

ocl_info *ocl;

//******************************************************************************

bool allocateHost(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
  unsigned ni, bool use_images, bool pinned);

//******************************************************************************

bool freeHost(
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
  bool use_images, bool pinned);

//******************************************************************************

bool allocateDevice(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
  unsigned ni, bool use_images);

//******************************************************************************

bool freeDevice(
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
  bool use_images);

//******************************************************************************

bool fillRand(std::vector<unsigned char*> &host_raw, size_t img_len);

//******************************************************************************

bool copyHostToDev(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
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

  std::vector<cl::Image2D*>   host_img;
  std::vector<cl::Buffer*>    host_buf;
  std::vector<unsigned char*> host_raw;
  std::vector<cl::Image2D*>   dev_img;
  std::vector<cl::Buffer*>    dev_buf;

  std::cout << "\n\nAlocating host memory..." << std::flush;
  ret = allocateHost(iw, ih, ip, ie, host_img, host_buf, host_raw,
    ni, use_images, pinned);
  std::cout << "done" << std::endl;

  std::cout << "Randomizing host memory..." << std::flush;
  ret = fillRand(host_raw, iw*ih*ip*ie);
  std::cout << "done" << std::endl;

  std::cout << "Alocating device memory..." << std::flush;
  ret = allocateDevice(iw, ih, ip, ie, dev_img, dev_buf, ni, use_images);
  std::cout << "done" << std::endl;

  std::cout << "Copying host to device..." << std::flush;
  ocl->q.finish();

  boost::chrono::system_clock::time_point t_start = boost::chrono::system_clock::now();
  ret = copyHostToDev(iw, ih, ip, ie,
    host_img, host_buf, host_raw,
    dev_img, dev_buf,
    use_images, mapped,
    nj);
  boost::chrono::system_clock::time_point t_stop = boost::chrono::system_clock::now();
  std::cout << "done" << std::endl;

  std::cout << "Freeing device memory..." << std::flush;
  ret = freeDevice(dev_img, dev_buf, use_images);
  std::cout << "done" << std::endl;

  std::cout << "Freeing host memory..." << std::flush;
  ret = freeHost(host_img, host_buf, host_raw, use_images, pinned);
  std::cout << "done" << std::endl;

  // Convert time struct to total seconds

  boost::chrono::duration<double> sec = t_stop - t_start;
  double t_seconds = sec.count();
  std::cout << "\n\nImage size: " << iw << 'x' << ih << 'x' << ip << " ("
           << ((iw*ih*ip*ie) / (1024.0*1024)) << "MB)" << std::endl;
  std::cout << ni*nj << " seperate transfers" << std::endl;
  std::cout << t_seconds << "s total" << std::endl;
  std::cout << (t_seconds/(ni*nj)) << "s per image" << std::endl;
  double img_size = iw*ih*ip*ie;
  double total_size = img_size*ni*nj;
  std::cout << (total_size/t_seconds)/(1024.0*1024.0) << " MB/s" << std::endl;

  delete ocl;

  return EXIT_FAILURE;
}

//******************************************************************************

bool allocateHost(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
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
      std::cerr << "Unsupported number of planes" << std::endl;
      return false;
  }

  cl_channel_type ct;
  switch(ie)
  {
    case 1: ct = CL_UNORM_INT8; break;
    case 2: ct = CL_UNORM_INT16; break;
    case 4: ct = CL_FLOAT; break;
    default:
      std::cerr << "Unsupported image element size" << std::endl;
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
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
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
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
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
      std::cerr << "Unsupported number of planes" << std::endl;
      return false;
  }

  cl_channel_type ct;
  switch(ie)
  {
    case 1: ct = CL_UNORM_INT8; break;
    case 2: ct = CL_UNORM_INT16; break;
    case 4: ct = CL_FLOAT; break;
    default:
      std::cerr << "Unsupported image element size" << std::endl;
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
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
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

bool fillRand(std::vector<unsigned char*> &host_raw, size_t img_len)
{
  std::srand(std::time(NULL));

  size_t len_4 = img_len / sizeof(int);

  for(size_t j = 0; j < host_raw.size(); ++j)
  {
    int *ptr = reinterpret_cast<int*>(host_raw[j]);
    for(size_t i = 0; i < len_4; ++i)
    {
      ptr[i] = std::rand();
    }
  }
  return true;
}

//******************************************************************************

bool copyHostToDev(unsigned iw, unsigned ih, unsigned ip, unsigned ie,
  std::vector<cl::Image2D*>   &host_img,
  std::vector<cl::Buffer*>    &host_buf,
  std::vector<unsigned char*> &host_raw,
  std::vector<cl::Image2D*>   &dev_img,
  std::vector<cl::Buffer*>    &dev_buf,
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

          std::memcpy(dev_raw, host_raw[i], len);

          ocl->q.enqueueUnmapMemObject(*dev_img[i], dev_raw);
        }
        else
        {
          unsigned char *dev_raw = reinterpret_cast<unsigned char*>(
            ocl->q.enqueueMapBuffer(*dev_buf[i], CL_TRUE, CL_MAP_WRITE,
              0, len));

          std::memcpy(dev_raw, host_raw[i], len);

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

