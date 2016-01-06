/* Exit status: 0 for a normal exit, 1 for environmental problems
  (file not found, invalid flags, I/O errors, etc), 2 to indicate a
  corrupt or invalid input file, 3 for an internal consistency error
  (eg, bug) which caused arg_parser to panic.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"


static const char * const Program_name = "Arg_parser";
static const char * const program_name = "arg_parser";
static const char * const program_year = "2015";
static const char * invocation_name = 0;


void show_help( const char verbose )
  {
  printf( "%s - POSIX/GNU command line argument parser. (C version)\n", Program_name );
  printf( "See the source file 'cmain.c' to learn how to use %s in\n", Program_name );
  printf( "your own programs.\n"
          "\nUsage: %s [options]\n", invocation_name );
  printf( "\nOptions:\n"
          "  -h, --help                   display this help and exit\n"
          "  -V, --version                output version information and exit\n"
          "  -a, --append                 example of option with no argument\n"
          "  -b, --block=<arg>            example of option with required argument\n"
          "  -c, --casual[=<arg>]         example of option with optional argument\n"
          "  -o <arg>                     example of short only option\n"
          "      --orphan                 example of long only option\n"
          "  -q, --quiet                  quiet operation\n"
          "  -u, --uncaught               example of intentional bug\n"
          "  -v, --verbose                verbose operation\n" );
  if( verbose )
    {
    printf( "  -H, --hidden                 example of hidden option (shown with -v -h)\n" );
    }
  printf( "\nReport bugs to arg-parser-bug@nongnu.org\n"
          "Arg_parser home page: http://www.nongnu.org/arg-parser/arg_parser.html\n" );
  }


void show_version()
  {
  printf( "%s \n", program_name );
  printf( "Copyright (C) %s Antonio Diaz Diaz.\n", program_year );
  printf( "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
          "This is free software: you are free to change and redistribute it.\n"
          "There is NO WARRANTY, to the extent permitted by law.\n" );
  }


void show_error( const char * const msg, const int errcode, const char help )
  {
  if( msg && msg[0] )
    {
    fprintf( stderr, "%s: %s", program_name, msg );
    if( errcode > 0 )
      fprintf( stderr, ": %s.", strerror( errcode ) );
    fprintf( stderr, "\n" );
    }
  if( help )
    fprintf( stderr, "Try '%s --help' for more information.\n",
             invocation_name );
  }


void internal_error( const char * const msg )
  {
  fprintf( stderr, "%s: internal error: %s\n", program_name, msg );
  exit( 3 );
  }


const char * optname( const int code, const struct ap_Option options[] )
  {
  static char buf[2] = "?";
  int i;

  if( code != 0 )
    for( i = 0; options[i].code; ++i )
      if( code == options[i].code )
        { if( options[i].name ) return options[i].name; else break; }
  if( code > 0 && code < 256 ) buf[0] = code; else buf[0] = '?';
  return buf;
  }


int main( const int argc, const char * const argv[] )
  {
  char verbose = 0;
  const struct ap_Option options[] =
    {
    { 'a', "append",   ap_no    },
    { 'b', "block",    ap_yes   },
    { 'c', "casual",   ap_maybe },
    { 'h', "help",     ap_no    },
    { 'H', "hidden",   ap_no    },
    { 'o', 0,          ap_yes   },
    { 'q', "quiet",    ap_no    },
    { 'u', "uncaught", ap_no    },
    { 'v', "verbose",  ap_no    },
    { 'V', "version",  ap_no    },
    { 256, "orphan",   ap_no    },
    {   0, 0,          ap_no    } };

  struct Arg_parser parser;
  int argind;
  invocation_name = argv[0];

  if( !ap_init( &parser, argc, argv, options, 0 ) )
    { show_error( "Not enough memory.", 0, 0 ); return 1; }
  if( ap_error( &parser ) )				/* bad option */
    { show_error( ap_error( &parser ), 0, 1 ); return 1; }

  for( argind = 0; argind < ap_arguments( &parser ); ++argind )
    {
    const int code = ap_code( &parser, argind );
    if( !code ) break;				/* no more options */
    switch( code )
      {
      case 'a': break;				/* example, do nothing */
      case 'b': break;				/* example, do nothing */
      case 'c': break;				/* example, do nothing */
      case 'h': show_help( verbose ); return 0;
      case 'H': break;				/* example, do nothing */
      case 'o': break;				/* example, do nothing */
      case 'q': verbose = 0; break;
      /* case 'u': break; */			/* intentionally not caught */
      case 'v': verbose = 1; break;
      case 'V': show_version(); return 0;
      case 256: break;				/* example, do nothing */
      default : internal_error( "uncaught option." );
      }
    } /* end process options */

  for( argind = 0; argind < ap_arguments( &parser ); ++argind )
    {
    const int code = ap_code( &parser, argind );
    const char * const arg = ap_argument( &parser, argind );
    if( code )	/* option */
      {
      const char * const name = optname( code, options );
      if( !name[1] )
        printf( "option '-%c'", name[0] );
      else
        printf( "option '--%s'", name );
      if( arg[0] )
        printf( " with argument '%s'", arg );
      }
    else	/* non-option */
      printf( "non-option argument '%s'", arg );
    printf( "\n" );
    }

  if( !ap_arguments( &parser ) ) printf( "Hello, world!\n" );

  return 0;
  }
