/*
**      cdecl -- C gibberish translator
**      src/lang.c
**
**      Paul J. Lucas
*/

// local
#include "config.h"                     /* must go first */
#include "common.h"
#include "lang.h"
#include "util.h"

// system
#include <assert.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////

// extern constant definitions
c_lang_info_t const C_LANG_INFO[] = {
  { LANG_C_KNR,   "cknr"    },          // synonym for "knr"
  { LANG_C_KNR,   "knr"     },
  { LANG_C_KNR,   "knrc"    },          // synonym for "knr"
  { LANG_C_MAX,   "c"       },
  { LANG_C_89,    "c89"     },
  { LANG_C_95,    "c95"     },
  { LANG_C_99,    "c99"     },
  { LANG_C_11,    "c11"     },
  { LANG_CPP_MAX, "c++"     },
  { LANG_CPP_98,  "c++98"   },
  { LANG_CPP_03,  "c++03"   },
  { LANG_CPP_11,  "c++11"   },
  { LANG_NONE,    NULL      },
};

////////// extern functions ///////////////////////////////////////////////////

c_lang_t c_lang_find( char const *s ) {
  assert( s != NULL );
  for ( c_lang_info_t const *info = C_LANG_INFO; info->name; ++info ) {
    if ( strcasecmp( s, info->name ) == 0 )
      return info->lang;
  } // for
  return LANG_NONE;
}

char const* c_lang_name( c_lang_t lang ) {
  switch ( lang ) {
    case LANG_NONE  : return "";
    case LANG_C_KNR : return "K&R C";
    case LANG_C_89  : return "C89";
    case LANG_C_95  : return "C95";
    case LANG_C_99  : return "C99";
    case LANG_C_11  : return "C11";
    case LANG_CPP_98: return "C++98";
    case LANG_CPP_03: return "C++03";
    case LANG_CPP_11: return "C++11";
    default:
      INTERNAL_ERR( "\"%d\": unexpected value for lang\n", (int)lang );
  } // switch
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
