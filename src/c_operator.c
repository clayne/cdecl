/*
**      cdecl -- C gibberish translator
**      src/c_operator.c
**
**      Copyright (C) 2018-2021  Paul J. Lucas
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
 * Defines static data for C/C++ operators.
 */

// local
#include "pjl_config.h"                 /* must go first */
/// @cond DOXYGEN_IGNORE
#define C_OPERATOR_INLINE _GL_EXTERN_INLINE
/// @endcond
#include "c_operator.h"
#include "c_ast.h"
#include "c_lang.h"
#include "gibberish.h"
#include "literals.h"
#include "options.h"

// standard
#include <assert.h>

/// @cond DOXYGEN_IGNORE
#define MBR                       C_OP_MEMBER
#define OVER                      C_OP_OVERLOADABLE
#define UNL                       C_OP_PARAMS_UNLIMITED
#define XXX                       C_OP_NOT_OVERLOADABLE
/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * Array of `c_operator` for all C++ operators.
 */
static c_operator_t const C_OPERATOR[] = {
  { C_OP_NONE,            "",         XXX,  0, 0,   LANG_NONE         },
  { C_OP_NEW,             L_NEW,      OVER, 1, UNL, LANG_CPP_ANY      },
  { C_OP_NEW_ARRAY,       "new[]",    OVER, 1, UNL, LANG_CPP_ANY      },
  { C_OP_DELETE,          L_DELETE,   OVER, 1, UNL, LANG_CPP_ANY      },
  { C_OP_DELETE_ARRAY,    "delete[]", OVER, 1, UNL, LANG_CPP_ANY      },
  { C_OP_EXCLAM,          "!",        OVER, 0, 1,   LANG_CPP_ANY      },
  { C_OP_EXCLAM_EQ,       "!=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PERCENT,         "%",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PERCENT_EQ,      "%=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_AMPER,           "&",        OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_AMPER2,          "&&",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_AMPER_EQ,        "&=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PARENS,          "()",       MBR,  0, UNL, LANG_CPP_ANY      },
  { C_OP_STAR,            "*",        OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_STAR_EQ,         "*=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PLUS,            "+",        OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_PLUS2,           "++",       OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_PLUS_EQ,         "+=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_COMMA,           ",",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_MINUS,           "-",        OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_MINUS2,          "--",       OVER, 0, 2,   LANG_CPP_ANY      },
  { C_OP_MINUS_EQ,        "-=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_ARROW,           "->",       MBR,  0, 0,   LANG_CPP_ANY      },
  { C_OP_ARROW_STAR,      "->*",      OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_DOT,             ".",        XXX,  0, 0,   LANG_CPP_ANY      },
  { C_OP_DOT_STAR,        ".*",       XXX,  0, 0,   LANG_CPP_ANY      },
  { C_OP_SLASH,           "/",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_SLASH_EQ,        "/=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_COLON2,          "::",       XXX,  0, 0,   LANG_CPP_ANY      },
  { C_OP_LESS,            "<",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_LESS2,           "<<",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_LESS2_EQ,        "<<=",      OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_LESS_EQ,         "<=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_LESS_EQ_GREATER, "<=>",      OVER, 1, 2,   LANG_MIN(CPP_20)  },
  { C_OP_EQ,              "=",        MBR,  1, 1,   LANG_CPP_ANY      },
  { C_OP_EQ2,             "==",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_GREATER,         ">",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_GREATER_EQ,      ">=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_GREATER2,        ">>",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_GREATER2_EQ,     ">>=",      OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_QMARK_COLON,     "?:",       XXX,  0, 0,   LANG_CPP_ANY      },
  { C_OP_BRACKETS,        "[]",       MBR,  1, 1,   LANG_MAX(CPP_20)  },
  { C_OP_BRACKETS,        "[]",       MBR,  0, UNL, LANG_MIN(CPP_23)  },
  { C_OP_CIRC,            "^",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_CIRC_EQ,         "^=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PIPE,            "|",        OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PIPE_EQ,         "|=",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_PIPE2,           "||",       OVER, 1, 2,   LANG_CPP_ANY      },
  { C_OP_TILDE,           "~",        OVER, 0, 1,   LANG_CPP_ANY      },

  { (c_oper_id_t)(C_OP_TILDE + 1),
                          NULL,       XXX,  0, 0,   LANG_NONE         },
};

////////// local functions ////////////////////////////////////////////////////

/**
 * Gets the alternative token of a C++ operator \a token.
 *
 * @param token The C++ operator token to get the alternative token for.
 * @return If we're emitting alternative tokens and if \a token is a token that
 * has an alternative token, returns said token; otherwise returns \a token as-
 * is.
 */
PJL_WARN_UNUSED_RESULT
static char const* alt_token_c( char const *token ) {
  assert( token != NULL );

  if ( opt_alt_tokens ) {
    switch ( token[0] ) {
      case '!': switch ( token[1] ) {
                  case '=': return L_NOT_EQ;
                  default : return L_NOT;
                }
      case '&': switch ( token[1] ) {
                  case '&': return L_AND;
                  case '=': return L_AND_EQ;
                  default : return L_BITAND;
                } // switch
      case '|': switch ( token[1] ) {
                  case '|': return L_OR;
                  case '=': return L_OR_EQ;
                  default : return L_BITOR;
                } // switch
      case '~': return L_COMPL;
      case '^': switch ( token[1] ) {
                  case '=': return L_XOR_EQ;
                  default : return L_XOR;
                } // switch
    } // switch
  }

  return token;
}

////////// extern functions ///////////////////////////////////////////////////

c_operator_t const* c_oper_get( c_oper_id_t oper_id ) {
  assert( oper_id >= C_OP_NEW && oper_id <= C_OP_TILDE );

  c_operator_t const *best_op = NULL;

  //
  // We can't just use oper_id as a direct index since operator[] has multiple
  // entries, but we can start looking there.
  //
  for ( c_operator_t const *op = C_OPERATOR + oper_id; op->oper_id <= oper_id;
        ++op ) {
    if ( op->oper_id == oper_id ) {
      if ( opt_lang_is_any( op->lang_ids ) )
        return op;
      //
      // We found the operator, but the entry isn't supported for the current
      // language, so keep looking for one that is.  However, make a note of
      // the current entry and return it if we don't find a better entry since
      // we always have to return non-NULL.  The code in c_ast_check_oper()
      // will deal with an unsupported language.
      //
      best_op = op;
    }
  } // for

  if ( unlikely( best_op == NULL ) )
    INTERNAL_ERR( "%d: c_oper_get() didn't find operator\n", oper_id );

  return best_op;
}

char const* c_oper_token_c( c_oper_id_t oper_id ) {
  return alt_token_c( graph_token_c( c_oper_get( oper_id )->name ) );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
