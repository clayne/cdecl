/*
**      cdecl -- C gibberish translator
**      src/util.c
**
**      Copyright (C) 2017-2020  Paul J. Lucas, et al.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 * Defines utility functions.
 */

// local
#include "cdecl.h"                      /* must go first */
/// @cond DOXYGEN_IGNORE
#define C_UTIL_INLINE _GL_EXTERN_INLINE
/// @endcond
#include "slist.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#if HAVE_PWD_H
# include <pwd.h>                       /* for getpwuid() */
#endif /* HAVE_PWD_H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>                   /* for fstat() */
#include <sysexits.h>
#include <unistd.h>                     /* for geteuid() */

#ifdef HAVE_READLINE_READLINE_H
# include <readline/readline.h>
#endif /* HAVE_READLINE_READLINE_H */
#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif /* HAVE_READLINE_HISTORY_H */

#ifdef ENABLE_TERM_SIZE
# include <fcntl.h>                     /* for open(2) */
# if defined(HAVE_CURSES_H)
#   define _BOOL                        /* prevents clash of bool on Solaris */
#   include <curses.h>
#   undef _BOOL
# elif defined(HAVE_NCURSES_H)
#   include <ncurses.h>
# endif
# include <term.h>                      /* for setupterm(3) */
# include <unistd.h>                    /* for close(2) */
#endif /* ENABLE_TERM_SIZE */

/// @endcond

///////////////////////////////////////////////////////////////////////////////

// local variable definitions
static slist_t free_later_list;         ///< List of stuff to free later.

////////// local functions ////////////////////////////////////////////////////

#ifdef ENABLE_TERM_SIZE
/**
 * Gets a terminal capability value and checks it for an error.
 * If there is an error, prints an error message and exits.
 *
 * @param capname The name of the terminal capability.
 * @return Returns said value or 0 if it could not be determined.
 */
C_WARN_UNUSED_RESULT
static unsigned check_tigetnum( char const *capname ) {
  int const num = tigetnum( CONST_CAST(char*, capname) );
  if ( unlikely( num < 0 ) )
    PMESSAGE_EXIT( EX_UNAVAILABLE,
      "tigetnum(\"%s\") returned error code %d", capname, num
    );
  return (unsigned)num;
}
#endif /* ENABLE_TERM_SIZE */

////////// extern functions ///////////////////////////////////////////////////

char const* base_name( char const *path_name ) {
  assert( path_name != NULL );
  char const *const slash = strrchr( path_name, '/' );
  if ( slash != NULL )
    return slash[1] != '\0' ? slash + 1 : slash;
  return path_name;
}

void* check_realloc( void *p, size_t size ) {
  //
  // Autoconf, 5.5.1:
  //
  // realloc
  //    The C standard says a call realloc(NULL, size) is equivalent to
  //    malloc(size), but some old systems don't support this (e.g., NextStep).
  //
  if ( unlikely( size == 0 ) )
    size = 1;
  void *const r = p != NULL ? realloc( p, size ) : malloc( size );
  IF_EXIT( r == NULL, EX_OSERR );
  return r;
}

char* check_strdup( char const *s ) {
  if ( s == NULL )
    return NULL;
  char *const s_dup = strdup( s );
  IF_EXIT( s_dup == NULL, EX_OSERR );
  return s_dup;
}

#ifndef HAVE_FMEMOPEN
FILE* fmemopen( void *buf, size_t size, char const *mode ) {
  assert( buf != NULL );
  assert( mode != NULL );
  assert( strchr( mode, 'r' ) != NULL );
#ifdef NDEBUG
  (void)mode;
#endif /* NDEBUG */

  FILE *const tmp = tmpfile();
  IF_EXIT( tmp == NULL, EX_OSERR );
  if ( likely( size > 0 ) ) {
    IF_EXIT( fwrite( buf, 1, size, tmp ) != size, EX_IOERR );
    IF_EXIT( fseek( tmp, 0L, SEEK_SET ) != 0, EX_IOERR );
  }
  return tmp;
}
#endif /* HAVE_FMEMOPEN */

void* free_later( void *p ) {
  assert( p != NULL );
  slist_push_tail( &free_later_list, p );
  return p;
}

void free_now( void ) {
  slist_free( &free_later_list, NULL, &free );
}

#ifdef ENABLE_TERM_SIZE
void get_term_columns_lines( unsigned *ncolumns, unsigned *nlines ) {
  int         cterm_fd = -1;
  char        reason_buf[ 128 ];
  char const *reason = NULL;

  char const *const term = getenv( "TERM" );
  if ( unlikely( term == NULL ) ) {
    reason = "TERM environment variable not set";
    goto error;
  }

  char const *const cterm_path = ctermid( NULL );
  if ( unlikely( cterm_path == NULL || *cterm_path == '\0' ) ) {
    reason = "ctermid(3) failed to get controlling terminal";
    goto error;
  }

  if ( unlikely( (cterm_fd = open( cterm_path, O_RDWR )) == -1 ) ) {
    reason = STRERROR();
    goto error;
  }

  int sut_err;
  if ( setupterm( CONST_CAST(char*, term), cterm_fd, &sut_err ) == ERR ) {
    reason = reason_buf;
    switch ( sut_err ) {
      case -1:
        reason = "terminfo database not found";
        break;
      case 0:
        snprintf(
          reason_buf, sizeof reason_buf,
          "TERM=%s not found in database or too generic", term
        );
        break;
      case 1:
        reason = "terminal is harcopy";
        break;
      default:
        snprintf(
          reason_buf, sizeof reason_buf,
          "setupterm(3) returned error code %d", sut_err
        );
    } // switch
    goto error;
  }

  if ( ncolumns != NULL )
    *ncolumns = check_tigetnum( "cols" );
  if ( nlines != NULL )
    *nlines = check_tigetnum( "lines" );

error:
  if ( likely( cterm_fd != -1 ) )
    close( cterm_fd );
  if ( unlikely( reason != NULL ) )
    PMESSAGE_EXIT( EX_UNAVAILABLE,
      "failed to determine number of columns or lines in terminal: %s\n",
      reason
    );
}
#endif /* ENABLE_TERM_SIZE */

char const* home_dir( void ) {
  static char const *home;
  if ( home == NULL ) {
    home = getenv( "HOME" );
#if HAVE_GETEUID && HAVE_GETPWUID && HAVE_STRUCT_PASSWD_PW_DIR
    if ( home == NULL ) {
      struct passwd *const pw = getpwuid( geteuid() );
      if ( pw != NULL )
        home = pw->pw_dir;
    }
#endif /* HAVE_GETEUID && && HAVE_GETPWUID && HAVE_STRUCT_PASSWD_PW_DIR */
  }
  return home;
}

bool is_file( int fd ) {
  struct stat fd_stat;
  FSTAT( fd, &fd_stat );
  return S_ISREG( fd_stat.st_mode );
}

void perror_exit( int status ) {
  perror( me );
  exit( status );
}

char* read_input_line( char const *ps1, char const *ps2 ) {
  assert( ps1 != NULL );
  assert( ps2 != NULL );

  static char *buf;
  size_t buf_len = 0;

  free( buf );
  buf = NULL;

  for (;;) {
    static char *line;
#ifdef WITH_READLINE
    extern void readline_init( void );
    static bool called_readline_init;

    if ( !called_readline_init ) {
      readline_init();
      called_readline_init = true;
    }

    free( line );
    if ( (line = readline( buf ? ps2 : ps1 )) == NULL )
      goto check_for_error;
#else
    static size_t line_cap;
    PUTS_OUT( buf != NULL ? ps2 : ps1 );
    FFLUSH( stdout );
    if ( getline( &line, &line_cap, stdin ) == -1 )
      goto check_for_error;
#endif /* WITH_READLINE */

    if ( is_blank_line( line ) ) {
      if ( buf != NULL ) {
        //
        // If we've been accumulating continuation lines, a blank line ends it.
        //
        break;
      }
      continue;
    }

    size_t line_len = strlen( line );
    bool const is_continuation = ends_with_chr( line, line_len, '\\' );

    if ( is_continuation )
      line[ --line_len ] = '\0';        // get rid of '\'

    if ( buf == NULL ) {
      buf = check_strdup( line );
      buf_len = line_len;
    } else {
      size_t const new_len = buf_len + line_len;
      REALLOC( buf, char, new_len + 1/*null*/ );
      strcpy( buf + buf_len, line );
      buf_len = new_len;
    }

    if ( !is_continuation )
      break;
  } // for

  assert( buf != NULL );
  assert( *buf != '\0' );
#ifdef WITH_READLINE
  add_history( buf );
#endif /* WITH_READLINE */
  return buf;

check_for_error:
  FERROR( stdin );
  return NULL;
}

void path_append( char *path, char const *component ) {
  assert( path != NULL );
  assert( component != NULL );

  size_t const len = strlen( path );
  if ( len > 0 ) {
    path += len - 1;
    if ( *path != '/' )
      *++path = '/';
    strcpy( ++path, component );
  }
}

char* chrcpy_end( char *dst, char c ) {
  assert( dst != NULL );
  *dst = c;
  *++dst = '\0';
  return dst;
}

char* strcpy_end( char *dst, char const *src ) {
  assert( dst != NULL );
  assert( src != NULL );
  while ( (*dst++ = *src++) != '\0' )
    /* empty */;
  return dst - 1;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
