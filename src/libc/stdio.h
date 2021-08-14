/* <stdio.h> GCC libc header simplified */

#ifndef _STDIO_H
#define _STDIO_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <bits/types.h>
#include <bits/types/__FILE.h>
#include <bits/types/FILE.h>

#define _STDIO_USES_IOSTREAM

#include <bits/libio.h>

typedef _G_fpos_t fpos_t;


/* The possibilities for the third argument to `setvbuf'.  */
#define _IOFBF 0		/* Fully buffered.  */
#define _IOLBF 1		/* Line buffered.  */
#define _IONBF 2		/* No buffering.  */

/* Default buffer size.  */
#ifndef BUFSIZ
# define BUFSIZ _IO_BUFSIZ
#endif


/* End of file character.
   Some things throughout the library rely on this being -1.  */
#ifndef EOF
# define EOF (-1)
#endif

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */
#ifdef __USE_GNU
# define SEEK_DATA	3	/* Seek to next data.  */
# define SEEK_HOLE	4	/* Seek to next hole.  */
#endif

/* Standard streams.  */
extern struct _IO_FILE *stdin;		/* Standard input stream.  */
extern struct _IO_FILE *stdout;		/* Standard output stream.  */
extern struct _IO_FILE *stderr;		/* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr

/* Remove file FILENAME.  */
extern int remove (const char *__filename) __THROW;
/* Rename file OLD to NEW.  */
extern int rename (const char *__old, const char *__new) __THROW;

extern FILE *tmpfile (void) __wur;

extern char *tmpnam (char *__s) __THROW __wur;

extern int fclose (FILE *__stream);

extern int fflush (FILE *__stream);

extern FILE *fopen (const char *__restrict __filename,
		    const char *__restrict __modes) __wur;

extern FILE *freopen (const char *__restrict __filename,
		      const char *__restrict __modes,
		      FILE *__restrict __stream) __wur;

extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __THROW;

extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
		    int __modes, size_t __n) __THROW;

extern int fprintf (FILE * __stream,
		    const char * __format, ...);

extern int printf (const char * __format, ...);

extern int sprintf (char * __s,
		    const char * __format, ...);

extern int vfprintf (FILE * __s, const char * __format,
		     _G_va_list __arg);

extern int vprintf (const char * __format, _G_va_list __arg);

extern int vsprintf (char * __s, const char * __format,
		     _G_va_list __arg);

extern int snprintf (char * __s, size_t __maxlen,
		     const char * __format, ...);

extern int vsnprintf(char * __s, size_t __maxlen,
		      const char * __format, _G_va_list __arg);

extern int fscanf (FILE * __stream,
		   const char * __format, ...);

extern int scanf (const char * __format, ...);

extern int sscanf (const char * __s,
		   const char * __format, ...);

extern int vfscanf (FILE * __s, const char * __format,
		    _G_va_list __arg);

extern int vscanf (const char * __format, _G_va_list __arg);

extern int vsscanf (const char * __s,
		    const char * __format, _G_va_list __arg);

extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);

extern int getchar (void);

extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);

extern int putchar (int __c);

extern char *fgets (char * __s, int __n, FILE * __stream)
     __wur;

extern _IO_ssize_t getline (char ** __lineptr,
			    size_t * __n,
			    FILE * __stream) __wur;

extern int fputs (const char * __s, FILE * __stream);

extern int puts (const char *__s);

extern int ungetc (int __c, FILE *__stream);

extern size_t fread (void * __ptr, size_t __size,
		     size_t __n, FILE * __stream);

extern size_t fwrite (const void * __ptr, size_t __size,
		      size_t __n, FILE * __s);

extern int fseek (FILE *__stream, long int __off, int __whence);

extern long int ftell (FILE *__stream) __wur;

extern void rewind (FILE *__stream);

extern int fgetpos (FILE * __stream, fpos_t * __pos);

extern int fsetpos (FILE *__stream, const fpos_t *__pos);

extern void clearerr (FILE *__stream);

extern int feof (FILE *__stream);

extern int ferror (FILE *__stream);

extern void perror (const char *__s);

#endif