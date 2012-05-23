#include <vcl_iostream.h>

#include "cl_manager.h"

int main(int argc, char *argv[])
{

  cl_manager::inst()->report_system_specs();

  return 0;
}
