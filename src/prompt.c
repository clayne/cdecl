/*
**      cdecl -- C gibberish translator
**      src/prompt.c
**
**      Copyright (C) 2017-2024  Paul J. Lucas
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
 * Defines global variables and functions for the prompt.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "prompt.h"
#ifdef WITH_READLINE
#include "autocomplete.h"
#endif /* WITH_READLINE */
#include "c_lang.h"
#include "cdecl.h"
#include "color.h"
#include "options.h"
#include "strbuf.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdbool.h>
#include <stddef.h>                     /* for NULL */

#ifdef WITH_READLINE
# ifdef HAVE_READLINE_READLINE_H
#   include <readline/readline.h>       /* for rl_gnu_readline_p */
# endif /* HAVE_READLINE_READLINE_H */

# if !HAVE_DECL_RL_PROMPT_START_IGNORE
#   define RL_PROMPT_START_IGNORE   '\1'
#   define RL_PROMPT_END_IGNORE     '\2'
# endif /* !HAVE_DECL_RL_PROMPT_START_IGNORE */
#endif /* WITH_READLINE */

/// @endcond

/**
 * @addtogroup prompt-group
 * @{
 */

#ifdef WITH_READLINE
  /**
   * Appends the character to start or end ignoring of characters to the prompt
   * for length calculation by **readline**(3).
   *
   * @param SBUF A pointer to the \ref strbuf to use.
   * @param WHEN Either the literal `START` or `END`.
   */
# define RL_PROMPT_IGNORE(SBUF,WHEN) \
    strbuf_putc( (SBUF), RL_PROMPT_##WHEN##_IGNORE )
#else
# define RL_PROMPT_IGNORE(SBUF,WHEN) NO_OP
#endif /* WITH_READLINE */

///////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_IGNORE
/// Otherwise Doxygen generates two entries.

// extern variable definitions
char const         *cdecl_prompt[2];

/// @endcond

// local variable definitions
static strbuf_t     prompt_buf[2];      ///< Buffers for prompts.

////////// inline functions ///////////////////////////////////////////////////

/**
 * Gets whether the prompt can and should be printed in color.
 *
 * @return Returns `true` only if so.
 */
NODISCARD
static inline bool color_prompt( void ) {
  return  sgr_prompt != NULL
#ifdef WITH_READLINE
          && HAVE_GENUINE_GNU_READLINE
#endif /* WITH_READLINE */
          ;
}

////////// local functions ////////////////////////////////////////////////////

/**
 * Cleans-up prompt strings.
 */
static void prompt_cleanup( void ) {
  strbuf_cleanup( &prompt_buf[0] );
  strbuf_cleanup( &prompt_buf[1] );
}

/**
 * Creates a prompt.
 *
 * @param suffix The prompt suffix character to use.
 * @param sbuf A pointer to the strbuf to use.  It is initially reset.
 */
static void prompt_create( char suffix, strbuf_t *sbuf ) {
  strbuf_reset( sbuf );

  if ( color_prompt() ) {
    // LCOV_EXCL_START -- test output is not in color
    RL_PROMPT_IGNORE( sbuf, START );
    color_strbuf_start( sbuf, sgr_prompt );
    RL_PROMPT_IGNORE( sbuf, END );
    // LCOV_EXCL_STOP
  }

  strbuf_printf( sbuf, "%s%c", OPT_LANG_IS( C_ANY ) ? CDECL : CPPDECL, suffix );

  if ( color_prompt() ) {
    // LCOV_EXCL_START
    RL_PROMPT_IGNORE( sbuf, START );
    color_strbuf_end( sbuf, sgr_prompt );
    RL_PROMPT_IGNORE( sbuf, END );
    // LCOV_EXCL_STOP
  }

  strbuf_putc( sbuf, ' ' );
}

////////// extern functions ///////////////////////////////////////////////////

void cdecl_prompt_enable( void ) {
  if ( opt_prompt ) {
    cdecl_prompt[0] = prompt_buf[0].str;
    cdecl_prompt[1] = prompt_buf[1].str;
  } else {
    cdecl_prompt[0] = cdecl_prompt[1] = "";
  }
}

void cdecl_prompt_init( void ) {
  RUN_ONCE ATEXIT( &prompt_cleanup );

  prompt_create( '>', &prompt_buf[0] );
  prompt_create( '+', &prompt_buf[1] );
  cdecl_prompt_enable();
}

size_t cdecl_prompt_len( void ) {
  if ( !opt_prompt )
    return 0;
  return (OPT_LANG_IS( C_ANY ) ? STRLITLEN( CDECL ) : STRLITLEN( CPPDECL ))
         + STRLITLEN( "> " );
}

///////////////////////////////////////////////////////////////////////////////

/** @} */

/* vim:set et sw=2 ts=2: */
