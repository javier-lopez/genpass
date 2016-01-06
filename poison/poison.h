/*
poison.h - header to ban unsafe C/C++ functions from applications, modified
License: public domain

Original: https://github.com/leafsr/gcc-poison, version 04.12.2013
Post about it: http://blog.leafsr.com/2013/12/02/gcc-poison
Copyright 2013 - Leaf Security Research
*/

//macro guard used to avoid the problem of double inclusion
#ifndef __POISON_H__
#define __POISON_H__
#ifdef __GNUC__

/* string handling functions */
#	pragma GCC poison strcpy wcscpy stpcpy wcpcpy
#	pragma GCC poison scanf sscanf vscanf fwscanf swscanf wscanf
#	pragma GCC poison gets puts
#	pragma GCC poison strcat wcscat
#	pragma GCC poison wcrtomb wctob
#	pragma GCC poison sprintf vsprintf vfprintf
#	pragma GCC poison asprintf vasprintf
#	pragma GCC poison strncpy wcsncpy
#	pragma GCC poison strtok wcstok
#	pragma GCC poison strdupa strndupa

/* signal related */
#	pragma GCC poison longjmp siglongjmp
#	pragma GCC poison setjmp sigsetjmp

/* memory allocation */
///next line is commented because it produces "warning: poisoning existing macro "alloca"
//[enabledby default]" after including some system header
//#	pragma GCC poison alloca
#	pragma GCC poison mallopt

/* file API */
#	pragma GCC poison remove
#	pragma GCC poison mktemp tmpnam tempnam
#	pragma GCC poison getwd

/* misc */
#	pragma GCC poison getlogin getpass cuserid
#	pragma GCC poison rexec rexec_af

/* command and program execution */
//use of that functions is often insecure and unjustified
//system() call is used in cpwd so it was remowed from line below
#	pragma GCC poison exec execl execlp execle execv execvp execvpe execve fexecve

/* deprecated network functions */
#	pragma GCC poison gethostbyname gethostbyaddr inet_ntoa inet_aton

#endif
#endif
