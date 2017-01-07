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

/* CHUNK is the size of the memory chunk used by the zlib routines. */

#define CHUNK 0x4000

/* The following macro calls a zlib routine and checks the return
value. If the return value ("status") is not OK, it prints an error
message and exits the program. Zlib's error statuses are all less
than zero. */

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stderr,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            exit (EXIT_FAILURE);                                        \
        }                                                               \
    }

/* if "test" is true, print an error message and halt execution. */

#define FAIL(test,message) {                             \
        if (test) {                                      \
            inflateEnd (& strm);                         \
            fprintf (stderr, "%s:%d: " message           \
                     " file '%s' failed: %s\n",          \
                     __FILE__, __LINE__, file_name,      \
                     strerror (errno));                  \
            exit (EXIT_FAILURE);                         \
        }                                                \
    }

/* These are parameters to inflateInit2. See
http://zlib.net/manual.html for the exact meanings. */

#define windowBits 15
#define ENABLE_ZLIB_GZIP 32

int inf(FILE *source, FILE *dest)
{
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  //ret = inflateInit(&strm);
  ret = inflateInit2(&strm, 32 + MAX_WBITS);

  if (ret != Z_OK)
    return ret;

  /* decompress until deflate stream ends or end of file */
  do {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)inflateEnd(&strm);
      return Z_ERRNO;
    }
    if (strm.avail_in == 0)
      break;
    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return ret;
      }
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        (void)inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  (void)inflateEnd(&strm);
  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int unzipfile(const std::string& file_name, const std::string& output_name)
{
  FILE* fin = fopen(file_name.c_str(), "rb");
  FILE* fout = fopen(output_name.c_str(), "wb");
  int ret = inf(fin, fout);
  fclose(fin);
  fclose(fout);
  //if (ret != Z_OK)
  //  zerr(ret);
  return ret;

}
int unzipfile2(const std::string& file_name, const std::string& output_name)
{
  //const char * file_name = "test.gz";
  FILE * file;
  z_stream strm = { 0 };
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = in;
  strm.avail_in = 0;
  //CALL_ZLIB(inflateInit2(&strm, windowBits | ENABLE_ZLIB_GZIP));
  CALL_ZLIB(inflateInit2(&strm, 32 + MAX_WBITS));

  /* Open the file. */

  file = fopen(file_name.c_str(), "rb");
  FILE* outFile = fopen(output_name.c_str(), "wb");
  FAIL(!file, "open");
  while (1) {
    int bytes_read;
    //inflateInit2(&stream, 16 + MAX_WBITS);

    bytes_read = fread(in, sizeof(char), sizeof(in), file);
    FAIL(ferror(file), "read");
    strm.avail_in = bytes_read;
    do {
      unsigned have;
      strm.avail_out = CHUNK;
      strm.next_out = out;
      CALL_ZLIB(inflate(&strm, Z_NO_FLUSH));
      have = CHUNK - strm.avail_out;
      //fwrite(out, sizeof(unsigned char), have, stdout);
      fwrite(out, sizeof(unsigned char), have, outFile);
    } while (strm.avail_out == 0);
    if (feof(file)) {
      inflateEnd(&strm);
      break;
    }
  }
  FAIL(fclose(file), "close");
  fclose(outFile);
  return 0;
}


///* zpipe.c: example of proper use of zlib's inflate() and deflate()
//Not copyrighted -- provided to the public domain
//Version 1.4  11 December 2005  Mark Adler */
//
///* Version history:
//1.0  30 Oct 2004  First version
//1.1   8 Nov 2004  Add void casting for unused return values
//Use switch statement for inflate() return values
//1.2   9 Nov 2004  Add assertions to document zlib guarantees
//1.3   6 Apr 2005  Remove incorrect assertion in inf()
//1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
//Avoid some compiler warnings for input and output buffers
//*/
//
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include "zlib.h"
//
//#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
//#  include <fcntl.h>
//#  include <io.h>
//#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
//#else
//#  define SET_BINARY_MODE(file)
//#endif
//
//#define CHUNK 16384
//
///* Compress from file source to file dest until EOF on source.
//def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
//allocated for processing, Z_STREAM_ERROR if an invalid compression
//level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
//version of the library linked do not match, or Z_ERRNO if there is
//an error reading or writing the files. */
//int def(FILE *source, FILE *dest, int level)
//{
//  int ret, flush;
//  unsigned have;
//  z_stream strm;
//  unsigned char in[CHUNK];
//  unsigned char out[CHUNK];
//
//  /* allocate deflate state */
//  strm.zalloc = Z_NULL;
//  strm.zfree = Z_NULL;
//  strm.opaque = Z_NULL;
//  ret = deflateInit(&strm, level);
//  if (ret != Z_OK)
//    return ret;
//
//  /* compress until end of file */
//  do {
//    strm.avail_in = fread(in, 1, CHUNK, source);
//    if (ferror(source)) {
//      (void)deflateEnd(&strm);
//      return Z_ERRNO;
//    }
//    flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
//    strm.next_in = in;
//
//    /* run deflate() on input until output buffer not full, finish
//    compression if all of source has been read in */
//    do {
//      strm.avail_out = CHUNK;
//      strm.next_out = out;
//      ret = deflate(&strm, flush);    /* no bad return value */
//      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
//      have = CHUNK - strm.avail_out;
//      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
//        (void)deflateEnd(&strm);
//        return Z_ERRNO;
//      }
//    } while (strm.avail_out == 0);
//    assert(strm.avail_in == 0);     /* all input will be used */
//
//                                    /* done when last data in file processed */
//  } while (flush != Z_FINISH);
//  assert(ret == Z_STREAM_END);        /* stream will be complete */
//
//                                      /* clean up and return */
//  (void)deflateEnd(&strm);
//  return Z_OK;
//}
//
///* Decompress from file source to file dest until stream ends or EOF.
//inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
//allocated for processing, Z_DATA_ERROR if the deflate data is
//invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
//the version of the library linked do not match, or Z_ERRNO if there
//is an error reading or writing the files. */
//int inf(FILE *source, FILE *dest)
//{
//  int ret;
//  unsigned have;
//  z_stream strm;
//  unsigned char in[CHUNK];
//  unsigned char out[CHUNK];
//
//  /* allocate inflate state */
//  strm.zalloc = Z_NULL;
//  strm.zfree = Z_NULL;
//  strm.opaque = Z_NULL;
//  strm.avail_in = 0;
//  strm.next_in = Z_NULL;
//  //ret = inflateInit(&strm);
//  ret = inflateInit2(&strm, 32 + MAX_WBITS);
//
//  if (ret != Z_OK)
//    return ret;
//
//  /* decompress until deflate stream ends or end of file */
//  do {
//    strm.avail_in = fread(in, 1, CHUNK, source);
//    if (ferror(source)) {
//      (void)inflateEnd(&strm);
//      return Z_ERRNO;
//    }
//    if (strm.avail_in == 0)
//      break;
//    strm.next_in = in;
//
//    /* run inflate() on input until output buffer not full */
//    do {
//      strm.avail_out = CHUNK;
//      strm.next_out = out;
//      ret = inflate(&strm, Z_NO_FLUSH);
//      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
//      switch (ret) {
//      case Z_NEED_DICT:
//        ret = Z_DATA_ERROR;     /* and fall through */
//      case Z_DATA_ERROR:
//      case Z_MEM_ERROR:
//        (void)inflateEnd(&strm);
//        return ret;
//      }
//      have = CHUNK - strm.avail_out;
//      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
//        (void)inflateEnd(&strm);
//        return Z_ERRNO;
//      }
//    } while (strm.avail_out == 0);
//
//    /* done when inflate() says it's done */
//  } while (ret != Z_STREAM_END);
//
//  /* clean up and return */
//  (void)inflateEnd(&strm);
//  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
//}
//
///* report a zlib or i/o error */
//void zerr(int ret)
//{
//  fputs("zpipe: ", stderr);
//  switch (ret) {
//  case Z_ERRNO:
//    if (ferror(stdin))
//      fputs("error reading stdin\n", stderr);
//    if (ferror(stdout))
//      fputs("error writing stdout\n", stderr);
//    break;
//  case Z_STREAM_ERROR:
//    fputs("invalid compression level\n", stderr);
//    break;
//  case Z_DATA_ERROR:
//    fputs("invalid or incomplete deflate data\n", stderr);
//    break;
//  case Z_MEM_ERROR:
//    fputs("out of memory\n", stderr);
//    break;
//  case Z_VERSION_ERROR:
//    fputs("zlib version mismatch!\n", stderr);
//  }
//}
//
///* compress or decompress from stdin to stdout */
//int main(int argc, char **argv)
//{
//  int ret;
//
//  /* avoid end-of-line conversions */
//  //SET_BINARY_MODE(stdin);
//  //SET_BINARY_MODE(stdout);
//  
//  /* do compression if no arguments */
//  if (argc == 3) {
//    //ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
//    FILE* fin = fopen(argv[1], "rb");
//    FILE* fout = fopen(argv[2], "wb");
//    def(fin, fout, Z_DEFAULT_COMPRESSION);
//    fclose(fin);
//    fclose(fout);
//    if (ret != Z_OK)
//      zerr(ret);
//    return ret;
//  }
//
//  /* do decompression if -d specified */
//  else if (argc == 4 && strcmp(argv[1], "-d") == 0) {
//    //ret = inf(stdin, stdout);
//    FILE* fin = fopen(argv[2], "rb");
//    FILE* fout = fopen(argv[3], "wb");
//    inf(fin, fout);
//    fclose(fin);
//    fclose(fout);
//    if (ret != Z_OK)
//      zerr(ret);
//    return ret;
//  }
//
//  /* otherwise, report usage */
//  else {
//    fputs("zpipe usage: zpipe [-d] source dest\n", stderr);
//    return 1;
//  }
//}