/*
**      cdecl -- C gibberish translator
**      src/set_options.c
**
**      Copyright (C) 2017-2023  Paul J. Lucas, et al.
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
 * Defines data and functions that implement the **cdecl** `set` command.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "set_options.h"
#include "c_lang.h"
#include "c_type.h"
#include "did_you_mean.h"
#include "options.h"
#include "print.h"
#include "prompt.h"
#include "slist.h"
#include "types.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <stdio.h>
#include <string.h>

/// @endcond

/**
 * @addtogroup set-options-group
 * @{
 */

///////////////////////////////////////////////////////////////////////////////

/**
 * Convenience `struct` for passing arguments to a `set_*()` function.
 */
struct set_option_fn_args {
  bool           opt_enabled;           ///< True if the option is enabled.
  c_loc_t const *opt_name_loc;          ///< The location of the option name.
  char const    *opt_value;             ///< The option value, if any.
  c_loc_t const *opt_value_loc;         ///< The location of \a opt_value.
};

// local functions
NODISCARD
static bool set_alt_tokens( set_option_fn_args_t const* ),
#ifdef YYDEBUG
            set_bison_debug( set_option_fn_args_t const* ),
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
            set_debug( set_option_fn_args_t const* ),
#endif /* ENABLE_CDECL_DEBUG */
            set_digraphs( set_option_fn_args_t const* ),
            set_east_const( set_option_fn_args_t const* ),
            set_english_types( set_option_fn_args_t const* ),
            set_echo_commands( set_option_fn_args_t const* ),
            set_explain_by_default( set_option_fn_args_t const* ),
            set_explicit_ecsu( set_option_fn_args_t const* ),
            set_explicit_int( set_option_fn_args_t const* ),
#ifdef ENABLE_FLEX_DEBUG
            set_flex_debug( set_option_fn_args_t const* ),
#endif /* ENABLE_FLEX_DEBUG */
            set_lang( set_option_fn_args_t const* ),
            set_lang_impl( char const* ),
            set_prompt( set_option_fn_args_t const* ),
            set_semicolon( set_option_fn_args_t const* ),
            set_trailing_return( set_option_fn_args_t const* ),
            set_trigraphs( set_option_fn_args_t const* ),
            set_using( set_option_fn_args_t const* );

///////////////////////////////////////////////////////////////////////////////

/**
 * **cdecl** `set` options.
 */
static set_option_t const SET_OPTIONS[] = {
  { "alt-tokens",         SET_OPTION_TOGGLE,   false, &set_alt_tokens         },
#ifdef YYDEBUG
  { "bison-debug",        SET_OPTION_TOGGLE,   false, &set_bison_debug        },
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
  { "debug",              SET_OPTION_TOGGLE,   false, &set_debug              },
#endif /* ENABLE_CDECL_DEBUG */
  { "digraphs",           SET_OPTION_AFF_ONLY, false, &set_digraphs           },
  { "graphs",             SET_OPTION_NEG_ONLY, false, &set_digraphs           },
  { "east-const",         SET_OPTION_TOGGLE,   false, &set_east_const         },
  { "echo-commands",      SET_OPTION_TOGGLE,   false, &set_echo_commands      },
  { "english-types",      SET_OPTION_TOGGLE,   false, &set_english_types      },
  { "explain-by-default", SET_OPTION_TOGGLE,   false, &set_explain_by_default },
  { "explicit-ecsu",      SET_OPTION_TOGGLE,   true,  &set_explicit_ecsu      },
  { "explicit-int",       SET_OPTION_TOGGLE,   true,  &set_explicit_int       },
#ifdef ENABLE_FLEX_DEBUG
  { "flex-debug",         SET_OPTION_TOGGLE,   false, &set_flex_debug         },
#endif /* ENABLE_FLEX_DEBUG */
  { "lang",               SET_OPTION_AFF_ONLY, true,  &set_lang               },
  { "prompt",             SET_OPTION_TOGGLE,   false, &set_prompt             },
  { "semicolon",          SET_OPTION_TOGGLE,   false, &set_semicolon          },
  { "trailing-return",    SET_OPTION_TOGGLE,   false, &set_trailing_return    },
  { "trigraphs",          SET_OPTION_AFF_ONLY, false, &set_trigraphs          },
  { "using",              SET_OPTION_TOGGLE,   false, &set_using              },
  { NULL,                 SET_OPTION_TOGGLE,   false, NULL                    }
};

/**
 * Always-enabled `set_*()` arguments used when re-setting options.
 */
static set_option_fn_args_t const enabled_args = {
  /*opt_enabled=*/true, NULL, NULL, NULL
};

////////// local functions ////////////////////////////////////////////////////

/**
 * Convenience function for getting `"no"` or not to print.
 *
 * @param enabled Whether a toggle option is enabled.
 * @return If \a enabled is `true`, returns `"  "` (two spaces); if `false`,
 * returns `"no"`.
 */
NODISCARD
static inline char const* maybe_no( bool enabled ) {
  return enabled ? "  " : "no";
}

/**
 * Prints the current option settings.
 */
static void print_options( void ) {
  PRINTF( "  %salt-tokens\n", maybe_no( opt_alt_tokens ) );
#ifdef YYDEBUG
  PRINTF( "  %sbison-debug\n", maybe_no( opt_bison_debug ) );
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
  PRINTF( "  %sdebug\n", maybe_no( opt_cdecl_debug ) );
#endif /* ENABLE_CDECL_DEBUG */
  PRINTF( "  %seast-const\n", maybe_no( opt_east_const ) );
  PRINTF( "  %secho-commands\n", maybe_no( opt_echo_commands ) );
  PRINTF( "  %senglish-types\n", maybe_no( opt_english_types ) );
  PRINTF( "  %sexplain-by-default\n", maybe_no( opt_explain ) );

  if ( opt_explicit_ecsu != TB_NONE ) {
    PUTS( "    explicit-ecsu=" );
    print_explicit_ecsu( stdout );
    PUTC( '\n' );
  } else {
    PUTS( "  noexplicit-ecsu\n" );
  }

  if ( any_explicit_int() ) {
    PUTS( "    explicit-int=" );
    print_explicit_int( stdout );
    PUTC( '\n' );
  } else {
    PUTS( "  noexplicit-int\n" );
  }

#ifdef ENABLE_FLEX_DEBUG
  PRINTF( "  %sflex-debug\n", maybe_no( opt_flex_debug ) );
#endif /* ENABLE_FLEX_DEBUG */
  PRINTF( " %sgraphs\n", opt_graph == C_GRAPH_DI ? " di" : opt_graph == C_GRAPH_TRI ? "tri" : " no" );
  PRINTF( "    lang=%s\n", c_lang_name( opt_lang ) );
  PRINTF( "  %sprompt\n", maybe_no( opt_prompt ) );
  PRINTF( "  %ssemicolon\n", maybe_no( opt_semicolon ) );
  PRINTF( "  %strailing-return\n", maybe_no( opt_trailing_ret ) );
  PRINTF( "  %susing\n", maybe_no( opt_using ) );
}

/**
 * Sets the alt-tokens option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_alt_tokens( set_option_fn_args_t const *args ) {
  opt_alt_tokens = args->opt_enabled;
  if ( opt_alt_tokens && !OPT_LANG_IS( ALT_TOKENS ) ) {
    print_warning( args->opt_name_loc,
      "alt-tokens not supported%s\n",
      C_LANG_WHICH( ALT_TOKENS )
    );
  }
  return true;
}

#ifdef YYDEBUG
/**
 * Sets the Bison debugging option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_bison_debug( set_option_fn_args_t const *args ) {
  opt_bison_debug = args->opt_enabled;
  return true;
}
#endif /* YYDEBUG */

#ifdef ENABLE_CDECL_DEBUG
/**
 * Sets the debug option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_debug( set_option_fn_args_t const *args ) {
  opt_cdecl_debug = args->opt_enabled;
  return true;
}
#endif /* ENABLE_CDECL_DEBUG */

/**
 * Sets the digraphs-tokens option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_digraphs( set_option_fn_args_t const *args ) {
  opt_graph = args->opt_enabled ? C_GRAPH_DI : C_GRAPH_NONE;
  if ( opt_graph && !OPT_LANG_IS( DIGRAPHS ) ) {
    print_warning( args->opt_name_loc,
      "digraphs not supported%s\n",
      C_LANG_WHICH( DIGRAPHS )
    );
  }
  return true;
}

/**
 * Sets the east-const option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_east_const( set_option_fn_args_t const *args ) {
  opt_east_const = args->opt_enabled;
  if ( opt_east_const && !OPT_LANG_IS( const ) ) {
    print_warning( args->opt_name_loc,
      "east-const not supported%s\n",
      C_LANG_WHICH( const )
    );
  }
  return true;
}

/**
 * Sets the echo-commands option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_echo_commands( set_option_fn_args_t const *args ) {
  opt_echo_commands = args->opt_enabled;
  if ( opt_echo_commands && cdecl_interactive ) {
    print_warning( args->opt_name_loc,
      "echo-commands has no effect when interactive\n"
    );
  }
  return true;
}

/**
 * Sets the english-types option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_english_types( set_option_fn_args_t const *args ) {
  opt_english_types = args->opt_enabled;
  return true;
}

/**
 * Sets the explain-by-default option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_explain_by_default( set_option_fn_args_t const *args ) {
  opt_explain = args->opt_enabled;
  return true;
}

/**
 * Sets the explicit-ecsu option.
 *
 * @param args The set option arguments.
 * @return Returns `true` only if the option was set.
 */
static bool set_explicit_ecsu( set_option_fn_args_t const *args ) {
  bool ok;

  if ( args->opt_enabled ) {
    ok = parse_explicit_ecsu( args->opt_value );
    if ( !ok ) {
      print_error( args->opt_value_loc,
        "\"%s\": invalid value for explicit-ecsu;"
        " must be a combination of e, c, s, or u\n",
        args->opt_value
      );
    }
  }
  else {
    ok = parse_explicit_ecsu( "" );
    assert( ok );
  }

  if ( ok && OPT_LANG_IS( C_ANY ) )
    print_warning( args->opt_name_loc, "explicit-ecsu is ignored in C\n" );

  return ok;
}

/**
 * Sets the explicit-int option.
 *
 * @param args The set option arguments.
 * @return Returns `true` only if the option was set.
 */
static bool set_explicit_int( set_option_fn_args_t const *args ) {
  bool ok;

  if ( args->opt_enabled ) {
    ok = parse_explicit_int( args->opt_value );
    if ( !ok ) {
      print_error( args->opt_value_loc,
        "\"%s\": invalid value for explicit-int;"
        " must be i, u, or {[u]{isl[l]}[,]}+\n",
        args->opt_value
      );
    }
  }
  else {
    ok = parse_explicit_int( "" );
    assert( ok );
  }

  return ok;
}

#ifdef ENABLE_FLEX_DEBUG
/**
 * Sets the Flex debugging option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_flex_debug( set_option_fn_args_t const *args ) {
  opt_flex_debug = args->opt_enabled;
  return true;
}
#endif /* ENABLE_FLEX_DEBUG */

/**
 * Sets the current language.
 *
 * @param args The set option arguments.
 * @return Returns `true` only if the option was set.
 */
static bool set_lang( set_option_fn_args_t const *args ) {
  assert( args->opt_enabled );
  if ( set_lang_impl( args->opt_value ) )
    return true;
  print_error( args->opt_value_loc,
    "\"%s\": unknown language\n", args->opt_value
  );
  return false;
}

/**
 * Sets the current language.
 *
 * @param name The language name.
 * @return Returns `true` only if \a name corresponds to a supported language
 * and the language was set.
 */
NODISCARD
static bool set_lang_impl( char const *name ) {
  c_lang_id_t const new_lang_id = c_lang_find( name );
  if ( new_lang_id == LANG_NONE )
    return false;
  c_lang_set( new_lang_id );
  //
  // Every time the language changes, re-set language-specific options so the
  // user is re-warned if the option is not supported in the current language.
  //
  if ( opt_alt_tokens )
    PJL_IGNORE_RV( set_alt_tokens( &enabled_args ) );
  switch ( opt_graph ) {
    case C_GRAPH_NONE:
      break;
    case C_GRAPH_DI:
      PJL_IGNORE_RV( set_digraphs( &enabled_args ) );
      break;
    case C_GRAPH_TRI:
      PJL_IGNORE_RV( set_trigraphs( &enabled_args ) );
      break;
  } // switch

  // set_east_const() isn't here since the only language it's not supported in
  // is K&RC and K&RC doesn't support const anyway.

  // set_using() isn't here since the feature is defined as printing types via
  // "using" declarations only in C++11 and later anyway.

  return true;
}

/**
 * Sets the prompt option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_prompt( set_option_fn_args_t const *args ) {
  opt_prompt = args->opt_enabled;
  cdecl_prompt_enable();
  return true;
}

/**
 * Sets the semicolon option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_semicolon( set_option_fn_args_t const *args ) {
  opt_semicolon = args->opt_enabled;
  return true;
}

/**
 * Sets the trailing return type option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_trailing_return( set_option_fn_args_t const *args ) {
  opt_trailing_ret = args->opt_enabled;
  if ( opt_trailing_ret && !OPT_LANG_IS( TRAILING_RETURN_TYPE ) ) {
    print_warning( args->opt_name_loc,
      "trailing return type not supported%s\n",
      C_LANG_WHICH( TRAILING_RETURN_TYPE )
    );
  }
  return true;
}

/**
 * Sets the trigraphs option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_trigraphs( set_option_fn_args_t const *args ) {
  opt_graph = args->opt_enabled ? C_GRAPH_TRI : C_GRAPH_NONE;
  if ( opt_graph && !OPT_LANG_IS( TRIGRAPHS ) ) {
    print_warning( args->opt_name_loc,
      "trigraphs not supported%s\n",
      C_LANG_WHICH( TRIGRAPHS )
    );
  }
  return true;
}

/**
 * Sets the using option.
 *
 * @param args The set option arguments.
 * @return Always returns `true`.
 */
static bool set_using( set_option_fn_args_t const *args ) {
  opt_using = args->opt_enabled;
  if ( opt_using && !OPT_LANG_IS( using_DECLARATION ) ) {
    print_warning( args->opt_name_loc,
      "using not supported%s\n",
      C_LANG_WHICH( using_DECLARATION )
    );
  }
  return true;
}

/**
 * Helper function for fprint_list() that, given a pointer to a pointer to an
 * slist_node whose data is a `set_option_t*`, returns the option's name.
 *
 * @param ppelt A pointer to the pointer to the element to get the string of.
 * On return, it is advanced to the next list element.
 * @return Returns said string or NULL if none.
 */
static char const* slist_set_option_gets( void const **ppelt ) {
  slist_node_t const *const node = *ppelt;
  if ( node == NULL )
    return NULL;
  set_option_t const *const opt = node->data;
  *ppelt = node->next;
  return opt->name;
}

/**
 * Compares strings for at most \a n characters ignoring hyphens for equality.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @param n The maximum number of characters to check.
 * @return Returns `true` only if \a s1 equals \a s2 (ignoring hyphens)
 * for \a n characters.
 */
NODISCARD
static bool strn_nohyphen_equal( char const *s1, char const *s2, size_t n ) {
  while ( n-- > 0 ) {
    if ( *s1 == '-' )
      ++s1;
    else if ( *s2 == '-' )
      ++s2;
    else if ( *s1++ != *s2++ )
      return false;
  } // while
  return true;
}

////////// extern functions ///////////////////////////////////////////////////

bool set_option( char const *opt_name, c_loc_t const *opt_name_loc,
                 char const *opt_value, c_loc_t const *opt_value_loc ) {
  if ( opt_name == NULL || strcmp( opt_name, "options" ) == 0 ) {
    print_options();
    return true;
  }

  if ( set_lang_impl( opt_name ) )
    return true;

  assert( opt_name_loc != NULL );
  assert( opt_value == NULL || opt_value_loc != NULL );

  char const *const orig_name = opt_name;
  bool const is_no = strncmp( opt_name, "no", 2 ) == 0;
  if ( is_no )
    opt_name += 2/*no*/;
  size_t const opt_name_len = strlen( opt_name );

  slist_t found_opt_list;
  slist_init( &found_opt_list );

  FOREACH_SET_OPTION( opt ) {
    if ( strn_nohyphen_equal( opt->name, opt_name, opt_name_len ) )
      slist_push_back( &found_opt_list, CONST_CAST( void*, opt ) );
  } // for

  switch ( slist_len( &found_opt_list ) ) {
    case 0:
      print_error( opt_name_loc, "\"%s\": unknown set option", orig_name );
      print_suggestions( DYM_SET_OPTIONS, orig_name );
      EPUTC( '\n' );
      return false;
    case 1:
      break;
    default:
      print_error( opt_name_loc,
        "\"%s\": ambiguous set option; could be ", orig_name
      );
      fprint_list( stderr, found_opt_list.head, &slist_set_option_gets );
      EPUTC( '\n' );
      slist_cleanup( &found_opt_list, /*free_fn=*/NULL );
      return false;
  } // switch

  set_option_t const *const found_opt = slist_front( &found_opt_list );
  slist_cleanup( &found_opt_list, /*free_fn=*/NULL );

  switch ( found_opt->kind ) {
    case SET_OPTION_TOGGLE:
      break;
    case SET_OPTION_AFF_ONLY:
      if ( is_no ) {
        print_error( opt_name_loc,
          "\"no\" not valid for \"%s\"\n", found_opt->name
        );
        return false;
      }
      break;
    case SET_OPTION_NEG_ONLY:
      if ( !is_no ) {
        print_error( opt_name_loc,
          "\"no\" required for \"%s\"\n", found_opt->name
        );
        return false;
      }
      break;
  } // switch

  if ( opt_value == NULL ) {
    if ( !is_no && found_opt->takes_value ) {
      print_error( opt_name_loc,
        "set option \"%s\" requires =<value>\n",
        orig_name
      );
      return false;
    }
  } else {
    if ( is_no ) {
      print_error( opt_value_loc, "\"no\" set options take no value\n" );
      return false;
    }
    if ( !found_opt->takes_value ) {
      print_error( opt_value_loc,
        "\"%s\": set option \"%s\" takes no value\n",
        opt_value, orig_name
      );
      return false;
    }
  }

  set_option_fn_args_t const args = {
    !is_no, opt_name_loc, opt_value, opt_value_loc
  };
  return (*found_opt->set_fn)( &args );
}

set_option_t const* set_option_next( set_option_t const *opt ) {
  return opt == NULL ? SET_OPTIONS : (++opt)->name == NULL ? NULL : opt;
}

///////////////////////////////////////////////////////////////////////////////

/** @} */

/* vim:set et sw=2 ts=2: */
