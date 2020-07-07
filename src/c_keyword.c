/*
**      cdecl -- C gibberish translator
**      src/c_keyword.c
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
 * Defines functions for looking up C/C++ keyword information.
 */

// local
#include "cdecl.h"                      /* must go first */
#include "c_keyword.h"
#include "c_ast.h"
#include "c_ast_util.h"
#include "c_lang.h"
#include "c_type.h"
#include "literals.h"
#include "parser.h"                     /* must go last */

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <string.h>

/// @endcond

#define LANG_C_CPP_11_MIN         LANG_C_CPP_MIN(11, 11)

///////////////////////////////////////////////////////////////////////////////

/**
 * Array of all C++ attributes.
 */
static c_keyword_t const C_ATTRIBUTES[] = {
  // C++11
  { L_CARRIES_DEPENDENCY,
                    Y_CARRIES_DEPENDENCY,
                                    T_CARRIES_DEPENDENCY,
                                                    LANG_CPP_MIN(11)  },
  { L_NORETURN,     Y_NORETURN,     T_NORETURN,     LANG_CPP_MIN(11)  },

  // C++14
  { L_DEPRECATED,   Y_DEPRECATED,   T_DEPRECATED,   LANG_CPP_MIN(14)  },

  // C++17
  { L_MAYBE_UNUSED, Y_MAYBE_UNUSED, T_MAYBE_UNUSED, LANG_CPP_MIN(17)  },
  { L_NODISCARD,    Y_NODISCARD,    T_NODISCARD,    LANG_CPP_MIN(17)  },

  // C++20                // Not implemented because:
#if 0
  { L_ASSERT,             // + These use arbitrary expressions that require
  { L_ENSURES,            //   being able to parse them -- which is a lot of
  { L_EXPECTS,            //   work for little benefit.

  { L_LIKELY,             // + These are only for statements, not declarations.
  { L_UNLIKELY,           //
#endif
  { L_NO_UNIQUE_ADDRESS,
                    Y_NO_UNIQUE_ADDRESS,
                                    T_NO_UNIQUE_ADDRESS,
                                                    LANG_CPP_MIN(20)  },

  { NULL,           0,              T_NONE,         LANG_NONE         }
};

/**
 * Array of all C/C++ keywords.
 *
 * @note There are two rows for `auto` since it has two meanings (one as a
 * storage class in C and C++ up to C++03 and the other as an automatically
 * deduced type in C++11 and later).
 */
static c_keyword_t const C_KEYWORDS[] = {
  // K&R C
  { L_AUTO,             Y_AUTO_STORAGE,     T_AUTO_STORAGE, LANG_MAX(CPP_03)  },
  { L_BREAK,            Y_BREAK,            T_NONE,         LANG_ALL          },
  { L_CASE,             Y_CASE,             T_NONE,         LANG_ALL          },
  { L_CHAR,             Y_CHAR,             T_CHAR,         LANG_ALL          },
  { L_CONTINUE,         Y_CONTINUE,         T_NONE,         LANG_ALL          },
  { L_DEFAULT,          Y_DEFAULT,          T_DEFAULT,      LANG_ALL          },
  { L_DO,               Y_DO,               T_DOUBLE,       LANG_ALL          },
  { L_DOUBLE,           Y_DOUBLE,           T_DOUBLE,       LANG_ALL          },
  { L_ELSE,             Y_ELSE,             T_NONE,         LANG_ALL          },
  { L_EXTERN,           Y_EXTERN,           T_EXTERN,       LANG_ALL          },
  { L_FLOAT,            Y_FLOAT,            T_FLOAT,        LANG_ALL          },
  { L_FOR,              Y_FOR,              T_NONE,         LANG_ALL          },
  { L_GOTO,             Y_GOTO,             T_NONE,         LANG_ALL          },
  { L_IF,               Y_IF,               T_NONE,         LANG_ALL          },
  { L_INT,              Y_INT,              T_INT,          LANG_ALL          },
  { L_LONG,             Y_LONG,             T_LONG,         LANG_ALL          },
  { L_REGISTER,         Y_REGISTER,         T_REGISTER,     LANG_ALL          },
  { L_RETURN,           Y_RETURN,           T_NONE,         LANG_ALL          },
  { L_SHORT,            Y_SHORT,            T_SHORT,        LANG_ALL          },
  { L_SIZEOF,           Y_SIZEOF,           T_NONE,         LANG_ALL          },
  { L_STATIC,           Y_STATIC,           T_STATIC,       LANG_ALL          },
  { L_STRUCT,           Y_STRUCT,           T_STRUCT,       LANG_ALL          },
  { L_SWITCH,           Y_SWITCH,           T_NONE,         LANG_ALL          },
  { L_TYPEDEF,          Y_TYPEDEF,          T_TYPEDEF,      LANG_ALL          },
  { L_UNION,            Y_UNION,            T_UNION,        LANG_ALL          },
  { L_UNSIGNED,         Y_UNSIGNED,         T_UNSIGNED,     LANG_ALL          },
  { L_WHILE,            Y_WHILE,            T_NONE,         LANG_ALL          },

  // C89
  { L_CONST,            Y_CONST,            T_CONST,        LANG_MIN(C_89)    },
  { L_ENUM,             Y_ENUM,             T_ENUM,         LANG_MIN(C_89)    },
  { L_SIGNED,           Y_SIGNED,           T_SIGNED,       LANG_MIN(C_89)    },
  { L_VOID,             Y_VOID,             T_VOID,         LANG_MIN(C_89)    },
  { L_VOLATILE,         Y_VOLATILE,         T_VOLATILE,     LANG_MIN(C_89)    },

  // C99
  { L__BOOL,            Y__BOOL,            T_BOOL,         LANG_MIN(C_99)    },
  { L__COMPLEX,         Y__COMPLEX,         T_COMPLEX,      LANG_C_MIN(99)    },
  { L__IMAGINARY,       Y__IMAGINARY,       T_IMAGINARY,    LANG_C_MIN(99)    },
  { L_INLINE,           Y_INLINE,           T_INLINE,       LANG_MIN(C_99)    },
  { L_RESTRICT,         Y_RESTRICT,         T_RESTRICT,     LANG_C_MIN(99)    },
  { L_WCHAR_T,          Y_WCHAR_T,          T_WCHAR_T,      LANG_MIN(C_95)    },

  // C11
  { L__ALIGNAS,         Y__ALIGNAS,         T_NONE,         LANG_C_MIN(11)    },
  { L__ALIGNOF,         Y__ALIGNOF,         T_NONE,         LANG_C_MIN(11)    },
  { L__ATOMIC,          Y__ATOMIC_QUAL,     T_ATOMIC,       LANG_MIN(C_11)    },
  { L__GENERIC,         Y__GENERIC,         T_NONE,         LANG_C_MIN(11)    },
  { L__NORETURN,        Y__NORETURN,        T_NORETURN,     LANG_C_MIN(11)    },
  { L__STATIC_ASSERT,   Y__STATIC_ASSERT,   T_NONE,         LANG_C_MIN(11)    },
  { L__THREAD_LOCAL,    Y__THREAD_LOCAL,    T_THREAD_LOCAL, LANG_C_MIN(11)    },

  // C++
  { L_BOOL,             Y_BOOL,             T_BOOL,         LANG_CPP_ALL      },
  { L_CATCH,            Y_CATCH,            T_NONE,         LANG_CPP_ALL      },
  { L_CLASS,            Y_CLASS,            T_CLASS,        LANG_CPP_ALL      },
  { L_CONST_CAST,       Y_CONST_CAST,       T_NONE,         LANG_CPP_ALL      },
  { L_DELETE,           Y_DELETE,           T_DELETE,       LANG_CPP_ALL      },
  { L_DYNAMIC_CAST,     Y_DYNAMIC_CAST,     T_NONE,         LANG_CPP_ALL      },
  { L_EXPLICIT,         Y_EXPLICIT,         T_EXPLICIT,     LANG_CPP_ALL      },
  { L_EXPORT,           Y_EXPORT,           T_NONE,         LANG_CPP_ALL      },
  { L_FALSE,            Y_FALSE,            T_NONE,         LANG_CPP_ALL      },
  { L_FRIEND,           Y_FRIEND,           T_FRIEND,       LANG_CPP_ALL      },
  { L_MUTABLE,          Y_MUTABLE,          T_MUTABLE,      LANG_CPP_ALL      },
  { L_NAMESPACE,        Y_NAMESPACE,        T_NAMESPACE,    LANG_CPP_ALL      },
  { L_NEW,              Y_NEW,              T_NONE,         LANG_CPP_ALL      },
  { L_OPERATOR,         Y_OPERATOR,         T_NONE,         LANG_CPP_ALL      },
  { L_PRIVATE,          Y_PRIVATE,          T_NONE,         LANG_CPP_ALL      },
  { L_PROTECTED,        Y_PROTECTED,        T_NONE,         LANG_CPP_ALL      },
  { L_PUBLIC,           Y_PUBLIC,           T_NONE,         LANG_CPP_ALL      },
  { L_REINTERPRET_CAST, Y_REINTERPRET_CAST, T_NONE,         LANG_CPP_ALL      },
  { L_STATIC_CAST,      Y_STATIC_CAST,      T_NONE,         LANG_CPP_ALL      },
  { L_TEMPLATE,         Y_TEMPLATE,         T_NONE,         LANG_CPP_ALL      },
  { L_THIS,             Y_THIS,             T_NONE,         LANG_CPP_ALL      },
  { L_THROW,            Y_THROW,            T_THROW,        LANG_CPP_ALL      },
  { L_TRUE,             Y_TRUE,             T_NOEXCEPT,     LANG_CPP_ALL      },
  { L_TRY,              Y_TRY,              T_NONE,         LANG_CPP_ALL      },
  { L_TYPEID,           Y_TYPEID,           T_NONE,         LANG_CPP_ALL      },
  { L_TYPENAME,         Y_TYPENAME,         T_NONE,         LANG_CPP_ALL      },
  { L_USING,            Y_USING,            T_TYPEDEF,      LANG_CPP_ALL      },
  { L_VIRTUAL,          Y_VIRTUAL,          T_VIRTUAL,      LANG_CPP_ALL      },

  // C++11
  { L_ALIGNAS,          Y_ALIGNAS,          T_NONE,         LANG_CPP_MIN(11)  },
  { L_ALIGNOF,          Y_ALIGNOF,          T_NONE,         LANG_CPP_MIN(11)  },
  { L_AUTO,             Y_AUTO_TYPE,        T_AUTO_TYPE,    LANG_CPP_MIN(11)  },
  { L_CONSTEXPR,        Y_CONSTEXPR,        T_CONSTEXPR,    LANG_CPP_MIN(11)  },
  { L_DECLTYPE,         Y_DECLTYPE,         T_NONE,         LANG_CPP_MIN(11)  },
  { L_FINAL,            Y_FINAL,            T_FINAL,        LANG_CPP_MIN(11)  },
  { L_NOEXCEPT,         Y_NOEXCEPT,         T_NOEXCEPT,     LANG_CPP_MIN(11)  },
  { L_NULLPTR,          Y_NULLPTR,          T_NONE,         LANG_CPP_MIN(11)  },
  { L_OVERRIDE,         Y_OVERRIDE,         T_OVERRIDE,     LANG_CPP_MIN(11)  },
  { L_STATIC_ASSERT,    Y_STATIC_ASSERT,    T_NONE,         LANG_CPP_MIN(11)  },
  { L_THREAD_LOCAL,     Y_THREAD_LOCAL,     T_THREAD_LOCAL, LANG_CPP_MIN(11)  },

  // C11 & C++11
  { L_CHAR16_T,         Y_CHAR16_T,         T_CHAR16_T,     LANG_C_CPP_11_MIN },
  { L_CHAR32_T,         Y_CHAR32_T,         T_CHAR32_T,     LANG_C_CPP_11_MIN },

  // C++20
  { L_CHAR8_T,          Y_CHAR8_T,          T_CHAR8_T,      LANG_CPP_MIN(20)  },
  { L_CONCEPT,          Y_CONCEPT,          T_NONE,         LANG_CPP_MIN(20)  },
  { L_CONSTEVAL,        Y_CONSTEVAL,        T_CONSTEVAL,    LANG_CPP_MIN(20)  },
  { L_REQUIRES,         Y_REQUIRES,         T_NONE,         LANG_CPP_MIN(20)  },

  // Alternative tokens
  { L_AND,              Y_AMPER2,           T_NONE,         LANG_MIN(C_89)    },
  { L_AND_EQ,           Y_AMPER_EQ,         T_NONE,         LANG_MIN(C_89)    },
  { L_BITAND,           Y_AMPER,            T_NONE,         LANG_MIN(C_89)    },
  { L_BITOR,            Y_PIPE,             T_NONE,         LANG_MIN(C_89)    },
  { L_COMPL,            Y_TILDE,            T_NONE,         LANG_MIN(C_89)    },
  { L_NOT,              Y_EXCLAM,           T_NONE,         LANG_MIN(C_89)    },
  { L_NOT_EQ,           Y_EXCLAM_EQ,        T_NONE,         LANG_MIN(C_89)    },
  { L_OR,               Y_PIPE2,            T_NONE,         LANG_MIN(C_89)    },
  { L_OR_EQ,            Y_PIPE_EQ,          T_NONE,         LANG_MIN(C_89)    },
  { L_XOR,              Y_CIRC,             T_NONE,         LANG_MIN(C_89)    },
  { L_XOR_EQ,           Y_CIRC_EQ,          T_NONE,         LANG_MIN(C_89)    },

  // GNU extensions
  { L_GNU___AUTO_TYPE,  Y_AUTO_TYPE,        T_AUTO_TYPE,    LANG_ALL          },
  { L_GNU___COMPLEX,    Y__COMPLEX,         T_COMPLEX,      LANG_ALL          },
  { L_GNU___COMPLEX__,  Y__COMPLEX,         T_COMPLEX,      LANG_ALL          },
  { L_GNU___CONST,      Y_CONST,            T_CONST,        LANG_ALL          },
  { L_GNU___INLINE,     Y_INLINE,           T_INLINE,       LANG_ALL          },
  { L_GNU___INLINE__,   Y_INLINE,           T_INLINE,       LANG_ALL          },
  { L_GNU___RESTRICT,   Y_GNU___RESTRICT,   T_RESTRICT,     LANG_ALL          },
  { L_GNU___RESTRICT__, Y_GNU___RESTRICT,   T_RESTRICT,     LANG_ALL          },
  { L_GNU___SIGNED,     Y_SIGNED,           T_SIGNED,       LANG_ALL          },
  { L_GNU___SIGNED__,   Y_SIGNED,           T_SIGNED,       LANG_ALL          },
  { L_GNU___THREAD,     Y_THREAD_LOCAL,     T_THREAD_LOCAL, LANG_ALL          },
  { L_GNU___VOLATILE,   Y_VOLATILE,         T_VOLATILE,     LANG_ALL          },
  { L_GNU___VOLATILE__, Y_VOLATILE,         T_VOLATILE,     LANG_ALL          },

  // Apple extensions
  { L_APPLE___BLOCK,    Y_APPLE___BLOCK,    T_APPLE_BLOCK,  LANG_ALL          },

  { NULL,               0,                  T_NONE,         LANG_NONE         }
};

////////// local functions ////////////////////////////////////////////////////

/**
 * Given a literal, gets the `c_keyword` for the corresponding C/C++ keyword in
 * \a lang_id.
 *
 * @param literal The literal to find.
 * @param keywords The array of `c_keyword` to look in.
 * @param lang_id The bitwise-or of language(s) to look for the keyword in.
 * @return Returns a pointer to the corresponding `c_keyword` or null for none.
 */
C_WARN_UNUSED_RESULT
static c_keyword_t const* c_keyword_find_impl( char const *literal,
                                               c_keyword_t const keywords[],
                                               c_lang_id_t lang_id ) {
  assert( literal != NULL );
  for ( c_keyword_t const *k = keywords; k->literal != NULL; ++k ) {
    if ( (k->lang_ids & lang_id) == LANG_NONE )
      continue;
    if ( strcmp( literal, k->literal ) == 0 )
      return k;
  } // for
  return NULL;
}

////////// extern functions ///////////////////////////////////////////////////

c_keyword_t const* c_attribute_find( char const *literal ) {
  return c_keyword_find_impl( literal, C_ATTRIBUTES, LANG_ALL );
}

c_keyword_t const* c_keyword_find( char const *literal, c_lang_id_t lang_id ) {
  return c_keyword_find_impl( literal, C_KEYWORDS, lang_id );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
