// <stdio.h> GCC libc header simplified
// Printf, file-handling etc

#ifndef _STDIO_H
#define _STDIO_H 1

#include <stddef.h>

/* The possibilities for the third argument to `setvbuf'.  */
#define _IOFBF 0 /* Fully buffered.  */
#define _IOLBF 1 /* Line buffered.  */
#define _IONBF 2 /* No buffering.  */

/* Default buffer size.  */
#ifndef BUFSIZ
    #define BUFSIZ _IO_BUFSIZ
#endif

/* End of file character.
   Some things throughout the library rely on this being -1.  */
#ifndef EOF
    #define EOF (-1)
#endif

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */

// #define _G_va_list __builtin_va_list

typedef struct _iobuf {
    char* _ptr;
    int _cnt;
    char* _base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char* _tmpfname;
} FILE;

// Standard streams.
extern struct FILE* stdin; /* Standard input stream.  */
extern struct FILE* stdout; /* Standard output stream.  */
extern struct FILE* stderr; /* Standard error output stream.  */

extern int remove(const char* __filename);

extern int rename(const char* __old, const char* __new);

extern FILE* tmpfile(void);

extern int fclose(FILE* __stream);

extern int fflush(FILE* __stream);

extern FILE* fopen(const char* __filename, const char* __modes);

extern FILE* freopen(const char* __filename, const char* __modes, FILE* __stream);

extern void setbuf(FILE* __stream, char* __buf);

extern int setvbuf(FILE* __stream, char* __buf, int __modes, size_t __n);

extern int fprintf(FILE* __stream, const char* __format, ...);

extern int printf(const char* __format, ...);

extern int sprintf(char* __s, const char* __format, ...);

//extern int vfprintf(FILE* __s, const char* __format, _G_va_list __arg);

//extern int vprintf(const char* __format, _G_va_list __arg);

//extern int vsprintf(char* __s, const char* __format, _G_va_list __arg);

extern int snprintf(char* __s, size_t __maxlen, const char* __format, ...);

//extern int vsnprintf(char* __s, size_t __maxlen, const char* __format, _G_va_list __arg);

extern int fscanf(FILE* __stream, const char* __format, ...);

extern int scanf(const char* __format, ...);

extern int sscanf(const char* __s, const char* __format, ...);

//extern int vfscanf(FILE* __s, const char* __format, _G_va_list __arg);

//extern int vscanf(const char* __format, _G_va_list __arg);

//extern int vsscanf(const char* __s, const char* __format, _G_va_list __arg);

extern int fgetc(FILE* __stream);
extern int getc(FILE* __stream);

extern int getchar(void);

extern int fputc(int __c, FILE* __stream);
extern int putc(int __c, FILE* __stream);

extern int putchar(int __c);

extern char* fgets(char* __s, int __n, FILE* __stream);

extern int getline(char** __lineptr, size_t* __n, FILE* __stream);

extern int fputs(const char* __s, FILE* __stream);

extern int puts(const char* __s);

extern int ungetc(int __c, FILE* __stream);

extern size_t fread(void* __ptr, size_t __size, size_t __n, FILE* __stream);

extern size_t fwrite(const void* __ptr, size_t __size, size_t __n, FILE* __s);

extern int fseek(FILE* __stream, long int __off, int __whence);

extern long int ftell(FILE* __stream);

extern void rewind(FILE* __stream);

extern void clearerr(FILE* __stream);

extern int feof(FILE* __stream);

extern int ferror(FILE* __stream);

extern void perror(const char* __s);

#endif
