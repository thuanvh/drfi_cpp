#pragma once
#include <io.h>
#include <stdio.h>
/* For "exit". */
#include <stdlib.h>
/* For "strerror". */
#include <string.h>
#include <string>
/* For "errno". */
#include <errno.h>
#include <zlib.h>
#include <assert.h>

class GzFileReader {
public:
  GzFileReader();
  ~GzFileReader();
  int open(const std::string& file);
  int read(void*  _Buffer, size_t _ElementSize, size_t _ElementCount);
private:
  unsigned char* in;
  unsigned char* out;
  unsigned out_idx;
  unsigned have;
  FILE *source;
  z_stream strm;  
};