/* Software License Agreement
 * 
 *     Copyright(C) 1994-2019 David Lindauer, (LADSoft)
 * 
 *     This file is part of the Orange C Compiler package.
 * 
 *     The Orange C Compiler package is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     The Orange C Compiler package is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Orange C.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *     As a special exception, if other files instantiate templates or
 *     use macros or inline functions from this file, or you compile
 *     this file and link it with other works to produce a work based
 *     on this file, this file does not by itself cause the resulting
 *     work to be covered by the GNU General Public License. However
 *     the source code for this file must still be made available in
 *     accordance with section (3) of the GNU General Public License.
 *     
 *     This exception does not invalidate any other reasons why a work
 *     based on this file might be covered by the GNU General Public
 *     License.
 * 
 *     contact information:
 *         email: TouchStone222@runbox.com <David Lindauer>
 * 
 */

/*  stdio.h

    Definitions for stream input/output.

*/

#ifndef __STDIO_H
#define __STDIO_H

#pragma pack(1)

#ifndef __STDDEF_H
#    include <stddef.h>
#endif

#ifndef __STDARG_H
#    include <stdarg.h>
#endif

#pragma pack(4)
typedef struct __file__
{
    short token;           /* Used for validity checking */
    unsigned short flags;  /* File status flags          */
    unsigned char hold;    /* Ungetc char if no buffer   */
    int fd;                /* File descriptor            */
    int level;             /* fill/empty level of buffer */
    int bsize;             /* Buffer size                */
    unsigned char* buffer; /* Data transfer buffer       */
    unsigned char* curp;   /* Current active pointer     */
    struct __file2
    {
        char* name; /* filename */
        enum
        {
            __or_unspecified,
            __or_narrow,
            __or_wide
        } orient;       /* stream orientation */
        int mbstate[2]; /* space for mbstate_t structure */
    } * extended;
} FILE; /* This is the FILE object    */
#pragma pack()

#if defined(__MSIL__)
#    if defined(__MANAGED__)
extern FILE _RTL_DATA* __stdin;
extern FILE _RTL_DATA* __stdout;
extern FILE _RTL_DATA* __stderr;
extern FILE _RTL_DATA* __stdaux;
extern FILE _RTL_DATA* __stdprn;
#    else
extern void* __stdin;
extern void* __stdout;
extern void* __stderr;
extern void* __stdaux;
extern void* __stdprn;
#    endif
#elif defined(__MSVCRT_DLL) || defined(__CRTDLL_DLL)
extern FILE* __stdin;
extern FILE* __stdout;
extern FILE* __stderr;
extern FILE* __stdaux;
extern FILE* __stdprn;
#elif defined(__LSCRTL_DLL)
extern FILE _IMPORT* __stdin;
extern FILE _IMPORT* __stdout;
extern FILE _IMPORT* __stderr;
extern FILE _IMPORT* __stdaux;
extern FILE _IMPORT* __stdprn;
#else
extern FILE _RTL_DATA* __stdin;
extern FILE _RTL_DATA* __stdout;
extern FILE _RTL_DATA* __stderr;
extern FILE _RTL_DATA* __stdaux;
extern FILE _RTL_DATA* __stdprn;
#endif

#define stdin __stdin
#define stdout __stdout
#define stderr __stderr
#define stdaux __stdaux /* these two not supported now */
#define stdprn __stdprn

#ifdef __cplusplus
extern "C"
{
#endif

    typedef long fpos_t;

#define FILTOK 0x444c
#define STACKPAD 512

/*  "flags" bits definitions
 */
#define _F_RDWR 0x0003          /* Read/write flag       */
#define _F_READ 0x0001          /* Read only file        */
#define _F_WRIT 0x0002          /* Write only file       */
#define _F_BUF 0x0004           /* Malloc'ed Buffer data */
#define _F_LBUF 0x0008          /* line-buffered file    */
#define _F_ERR 0x0010           /* Error indicator       */
#define _F_EOF 0x0020           /* End of file indicator */
#define _F_BIN 0x0040           /* Binary file indicator */
#define _F_IN 0x0080            /* Data is incoming      */
#define _F_OUT 0x0100           /* Data is outgoing      */
#define _F_TERM 0x0200          /* File is a terminal    */
#define _F_APPEND 0x400         /* Need to ignore seeks  */
#define _F_BUFFEREDSTRING 0x800 /* it is not an actual file */
#define _F_XEOF 0x1000          /* EOF is pending based on CTRL-Z in buffered input */
#define _F_VBUF 0x2000          /* true if setvbut allowed */
#define _F_UNGETC 0x4000        /* character was cached with ungetc */
/* End-of-file constant definition
 */
#define EOF (-1) /* End of file indicator */

/* Default buffer size use by "setbuf" function
 */
#define BUFSIZ 512 /* Buffer size for stdio */

/* Size of an arry large enough to hold a temporary file name string
 */
#define L_ctermid 5 /* CON: plus null byte */
#define P_tmpdir "" /* temporary directory */
#define L_tmpnam 13 /* tmpnam buffer size */

/* Constants to be used as 3rd argument for "fseek" function
 */
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

/* Number of unique file names that shall be generated by "tmpnam" function
 */
#define TMP_MAX 32767

#define _NFILE_ 40 /* SHOULD BE SAME AS IN IO.H */

/* Definition of the control structure for streams
 */

/* Number of files that can be open simultaneously
 */
#define FOPEN_MAX (_NFILE_ - 2) /* (_NFILE_ - stdaux & stdprn) */

#define FILENAME_MAX 265

    /* Standard I/O predefined streams
     */
#ifndef _NFILE_EXT
#    define _NFILE_EXT
    extern unsigned _RTL_DATA _nfile;
#endif
    FILE* _RTL_FUNC _IMPORT __getStream(int stream);

#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 4

    void _RTL_FUNC _IMPORT clearerr(FILE* __stream);
    int _RTL_FUNC _IMPORT fclose(FILE* __stream);
    int _RTL_FUNC _IMPORT fflush(FILE* __stream);
    int _RTL_FUNC _IMPORT fgetc(FILE* __stream);
    int _RTL_FUNC _IMPORT fgetpos(FILE* __stream, fpos_t* __pos);
    char* _RTL_FUNC _IMPORT fgets(char* __s, int __n, FILE* __stream);
    int _RTL_FUNC _IMPORT fileno(FILE*);
    int _RTL_FUNC _IMPORT _fileno(FILE*);
    FILE* _RTL_FUNC _IMPORT fopen(const char* __path, const char* __mode);
    int _RTL_FUNC _IMPORT fprintf(FILE* restrict __stream, const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT fputc(int __c, FILE* __stream);
    int _RTL_FUNC _IMPORT fputs(const char* __s, FILE* __stream);
    size_t _RTL_FUNC _IMPORT fread(void* __ptr, size_t __size, size_t __n, FILE* __stream);
    FILE* _RTL_FUNC _IMPORT freopen(const char* __path, const char* __mode, FILE* restrict __stream);
    int _RTL_FUNC _IMPORT fscanf(FILE* restrict __stream, const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT fseek(FILE* __stream, long __offset, int __whence);
    int _RTL_FUNC _IMPORT fsetpos(FILE* __stream, const fpos_t* __pos);
    long _RTL_FUNC _IMPORT ftell(FILE* __stream);
    size_t _RTL_FUNC _IMPORT fwrite(const void* __ptr, size_t __size, size_t __n, FILE* __stream);
    FILE* _RTL_FUNC popen(const char* name, const char* restrict mode);
    FILE* _RTL_FUNC _popen(const char* name, const char* restrict mode);
    char* _RTL_FUNC _IMPORT gets(char* __s);
    void _RTL_FUNC _IMPORT perror(const char* __s);
    int _RTL_FUNC _IMPORT printf(const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT puts(const char* __s);
    int _RTL_FUNC _IMPORT remove(const char* __path);
    int _RTL_FUNC _IMPORT rename(const char* __oldname, const char* __newname);
    void _RTL_FUNC _IMPORT rewind(FILE* __stream);
    int _RTL_FUNC _IMPORT scanf(const char* __format, ...);
    void _RTL_FUNC _IMPORT setbuf(FILE* restrict __stream, char* restrict __buf);
    int _RTL_FUNC _IMPORT setvbuf(FILE* restrict __stream, char* restrict __buf, int __type, size_t __size);
    int _RTL_FUNC _IMPORT snprintf(char* restrict __buffer, size_t n, const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT _snprintf(char* restrict __buffer, size_t n, const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT sprintf(char* restrict __buffer, const char* restrict __format, ...);
    int _RTL_FUNC _IMPORT sscanf(const char* restrict __buffer, const char* restrict __format, ...);
    char* _RTL_FUNC _IMPORT strerror(int __errnum);
    char* _RTL_FUNC _IMPORT tempnam(char* __dir, char* __prefix);
    char* _RTL_FUNC _IMPORT _tempnam(char* __dir, char* __prefix);
    FILE* _RTL_FUNC _IMPORT tmpfile(void);
    char* _RTL_FUNC _IMPORT tmpnam(char* __s);
    int _RTL_FUNC _IMPORT ungetc(int __c, FILE* __stream);
    int _RTL_FUNC _IMPORT vfprintf(FILE* restrict __stream, const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vfscanf(FILE* restrict __stream, const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vprintf(const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vscanf(const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vsnprintf(char* restrict __buffer, size_t __n, const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT _vsnprintf(char* restrict __buffer, size_t __n, const char* restrict __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vsprintf(char* __buffer, const char* __format, va_list __arglist);
    int _RTL_FUNC _IMPORT vsscanf(const char* __buffer, const char* __format, va_list __arglist);
    int _RTL_FUNC _IMPORT unlink(const char* __path);
    int _RTL_FUNC _IMPORT _unlink(const char* __path);
    int _RTL_FUNC _IMPORT getc(FILE* __fp);

    int _RTL_FUNC _IMPORT getchar(void);
    int _RTL_FUNC _IMPORT putchar(const int __c);

    int _RTL_FUNC _IMPORT putc(const int __c, FILE* __fp);
    int _RTL_FUNC _IMPORT feof(FILE* __fp);
    int _RTL_FUNC _IMPORT ferror(FILE* __fp);

    int _RTL_FUNC _IMPORT fcloseall(void);
    int _RTL_FUNC _IMPORT _fcloseall(void);
    FILE* _RTL_FUNC _IMPORT fdopen(int __handle, const char* restrict __type);
    FILE* _RTL_FUNC _IMPORT _fdopen(int __handle, const char* restrict __type);
    int _RTL_FUNC _IMPORT fgetchar(void);
    int _RTL_FUNC _IMPORT _fgetchar(void);
    int _RTL_FUNC _IMPORT flushall(void);
    int _RTL_FUNC _IMPORT _flushall(void);
    int _RTL_FUNC _IMPORT fputchar(int __c);
    int _RTL_FUNC _IMPORT _fputchar(int __c);
    FILE* _RTL_FUNC _IMPORT _fsopen(const char* __path, const char* __mode, int __shflag);
    int _RTL_FUNC _IMPORT getw(FILE* __stream);
    int _RTL_FUNC _IMPORT _getw(FILE* __stream);
    int _RTL_FUNC _IMPORT putw(int __w, FILE* __stream);
    int _RTL_FUNC _IMPORT _putw(int __w, FILE* __stream);
    int _RTL_FUNC _IMPORT rmtmp(void);
    int _RTL_FUNC _IMPORT _rmtmp(void);
    char* _RTL_FUNC _IMPORT _strerror(const char* __s);

#if !defined(__CRTDLL_DLL) && !defined(__MSVCRT_DLL) && !defined(__MSIL__)
#    define fileno(f) ((f)->fd)
#    define _fileno(f) ((f)->fd)
#endif
#if defined(__MSIL__)
#    define fileno(f) _fileno(f)
#endif
    int _RTL_FUNC _IMPORT _fgetc(FILE* __stream);          /* used by getc() macro */
    int _RTL_FUNC _IMPORT _fputc(int __c, FILE* __stream); /* used by putc() macro */

#ifdef __cplusplus
};
#endif

    /*  The following macros provide for common functions */

#if !defined(__CRTDLL_DLL) && !defined(__MSVCRT_DLL) && !defined(__MSIL__) && !defined(__LSCRTL_DLL)
#    define ferror(f) ((f)->flags & _F_ERR)
#    define feof(f) ((f)->flags & _F_EOF)
#endif
/*
 * the following four macros are somewhat problematic as they will
 * ignore line buffering...
 */
#define getc(f) fgetc(f)
#define putc(c, f) fputc((c), f)

#pragma pack()

#endif /* __STDIO_H */
