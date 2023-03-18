/*
**      cdecl -- C gibberish translator
**      src/gibberish.h
**
**      Copyright (C) 2019-2023  Paul J. Lucas
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

#ifndef cdecl_gibberish_H
#define cdecl_gibberish_H

/**
 * @file
 * Declares functions for printing in gibberish, aka, a C/C++ declaration.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "types.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdio.h>                      /* for FILE */

/// @endcond

/**
 * @defgroup printing-gibberish-group Printing Gibberish
 * Functions for printing in gibberish, aka, a C/C++ declaration.
 * @{
 */

// Flags for c_ast_gibberish() and c_typedef_gibberish().

/**
 * Unset value for gibberish flags for c_ast_gibberish() and
 * c_typedef_gibberish().  Can also be used to mean _not_ to print in gibberish
 * (hence, print in English).
 */
#define C_GIB_NONE        0u

/**
 * Flag for c_ast_gibberish() to print as a cast.
 *
 * @note May _not_ be used in combination with any other flags.
 *
 * @sa #C_GIB_DECL
 */
#define C_GIB_CAST        (1u << 0)

/**
 * Flag for c_ast_gibberish() to print as a declaration.
 *
 * @note May be used _only_ in combination with #C_GIB_FINAL_SEMI,
 * #C_GIB_MULTI_DECL, and #C_GIB_OMIT_TYPE.
 *
 * @sa #C_GIB_CAST
 * @sa #C_GIB_MULTI_DECL
 * @sa #C_GIB_OMIT_TYPE
 */
#define C_GIB_DECL        (1u << 1)

/**
 * Flag for c_ast_gibberish() or c_typedef_gibberish() to print the final
 * semicolon after a type declaration.
 *
 * @note May be used in combination with any other flags _except_ #C_GIB_CAST.
 */
#define C_GIB_FINAL_SEMI  (1u << 2)

/**
 * Flag for c_ast_gibberish() to indicate that the declaration is for multiple
 * types or objects, for example:
 *
 *      int *x, *y;
 *
 * @note Unlike #C_GIB_OMIT_TYPE, `C_GIB_MULTI_DECL` _must_ be used for the
 * entire declaration.
 * @note May be used _only_ in combination with #C_GIB_DECL and
 * #C_GIB_OMIT_TYPE.
 *
 * @sa #C_GIB_DECL
 * @sa #C_GIB_OMIT_TYPE
 */
#define C_GIB_MULTI_DECL  (1u << 3)

/**
 * Flag for c_ast_gibberish() to omit the type name when printing gibberish for
 * the _second_ and subsequent objects when printing multiple types or objects
 * in the same declaration.  For example, when printing:
 *
 *      int *x, *y;
 *
 * the gibberish for `y` _must not_ print the `int` again.
 *
 * @note May be used _only_ in combination with #C_GIB_DECL and
 * #C_GIB_MULTI_DECL.
 *
 * @sa #C_GIB_DECL
 * @sa #C_GIB_MULTI_DECL
 */
#define C_GIB_OMIT_TYPE   (1u << 4)

/**
 * Flag for c_typedef_gibberish() to print as a `typedef` declaration.
 *
 * @note May be used _only_ in combination with #C_GIB_FINAL_SEMI.
 *
 * @sa #C_GIB_USING
 */
#define C_GIB_TYPEDEF     (1u << 5)

/**
 * Flag for c_typedef_gibberish() to print as a `using` declaration.
 *
 * @note May be used _only_ in combination with #C_GIB_FINAL_SEMI.
 *
 * @sa #C_GIB_TYPEDEF
 */
#define C_GIB_USING       (1u << 6)

////////// extern functions ///////////////////////////////////////////////////

/**
 * Gets the alternative token of a C++ operator \a token.
 *
 * @param token The C++ operator token to get the alternative token for.
 * @return If we're emitting alternative tokens and if \a token is a token that
 * has an alternative token, returns said token; otherwise returns \a token as-
 * is.
 */
NODISCARD
char const* alt_token_c( char const *token );

/**
 * Prints \a ast as gibberish, aka, a C/C++ declaration or cast.
 *
 * @param ast The AST to print.
 * @param flags The gibberish flags to use; _must_ include either #C_GIB_CAST
 * or #C_GIB_DECL.
 * @param gout The `FILE` to print to.
 *
 * @sa c_ast_english()
 * @sa c_typedef_gibberish()
 */
void c_ast_gibberish( c_ast_t const *ast, unsigned flags, FILE *gout );

/**
 * Given \a kind, gets the associated C++ literal.
 *
 * @param kind The cast kind to get the literal for.  _Must only_ be one of
 * #C_CAST_CONST, #C_CAST_DYNAMIC, #C_CAST_REINTERPRET, or #C_CAST_STATIC.
 * @return Returns said literal.
 *
 * @sa c_cast_english()
 */
NODISCARD
char const* c_cast_gibberish( c_cast_kind_t kind );

/**
 * Prints \a tdef as a C/C++ type declaration.
 *
 * @param tdef The type to print.
 * @param flags The gibberish flags to use; _must_ include either
 * #C_GIB_TYPEDEF or #C_GIB_USING.
 * @param gout The `FILE` to print to.
 *
 * @sa c_ast_gibberish()
 * @sa c_typedef_english()
 * @sa print_type()
 */
void c_typedef_gibberish( c_typedef_t const *tdef, unsigned flags,
                          FILE *gout );

/**
 * Gets the digraph or trigraph (collectively, "graph") equivalent of \a token.
 *
 * @param token The token to get the graph token for.
 * @return If we're emitting graphs and \a token contains one or more
 * characters that have a graph equivalent, returns \a token with said
 * characters replaced by their graphs; otherwise returns \a token as-is.
 */
NODISCARD
char const* graph_token_c( char const *token );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_gibberish_H */
/* vim:set et sw=2 ts=2: */
