#ifndef STDIO_H
#define STDIO_H 1

#include "micro/file.h"

#define BUFSIZ 256

struct FILE;
typedef struct FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/*  Writes from array pointed to by ptr up to nmemb elements, whose size is specified by size
 *  to the stream pointed to by stream.
 *  ptr:    Buffer to write
 *  size:   Size of the elements to write
 *  nmemb:  Number of elements to write
 *  stream: Stream to output results
 *  r:      Number of bytes written
 * */
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);

int putc(const char c);
int fputc(FILE *restrict stream, const char c);

/*  Writes the string s and a trailing newline to stdout
 *  s:  Buffer to print
 *  r:  Non-negative on success, EOF on error
 * */
int puts(const char *s);

/*  Same as puts, except with output directed to stream
 *  stream: Output
 *  s:      Buffer to print
 *  r:      Non-negative on success, EOF on error
 * */
int fputs(FILE *restrict stream, const char *s);

/*  Writes output to stdout, under control of the string pointed to by
 *  format that specifies how subsequent arguments are converted for output
 *  If there are insufficient arguments, the behavior is undefined
 *  format: Control
 *  args:   Arguments to print
 *  r:      Error code if unsucessful, printed count if sucessful
 * */
int printf(const char *restrict format, ...);

/*  Same as printf, except output is directed towards stream
 *  stream: Stream to write data
 *  format: Control
 *  args:   Arguments to print
 *  r:      Erorr code if unsucessful, printed count if sucessful
 * */
int fprintf(FILE *restrict stream, const char *restrict format, ...);

/*  Same as fprintf, but writes to specific file descriptor
 *  fd:     File descriptor to write data
 *  format: Control
 *  args:   Arguments to print
 *  r:      Error code if unsucessful, printed count if sucessful
 * */
int dprintf(int fd, const char *restrict format, ...);

/*  Same as printf, except output is directed to a string
 *  str:    Output buffer
 *  format: Control
 *  args:   Arguments to print
 *  r:      Error code if unsucessful, printed count if sucessful
 * */
int sprintf(char *restrict str, const char *restrict format, ...);

/*  Same as sprintf, but output stops after n bytes
 *  str:    Output buffer
 *  size:   Number of bytes to write
 *  format: Control
 *  args:   Arguments to print
 *  r:      Error code if unsucessful, printed count if sucessful
 * */
int snprintf(char *restrict str, size_t size, const char *restrict format, ...);

#include <stdarg.h>

int vprintf(const char *restrict format, va_list ap);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap);
int vsprintf(char *restrict str, const char *restrict format, va_list ap);

#endif
