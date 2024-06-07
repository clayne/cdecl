/*
**      cdecl -- C gibberish translator
**      src/c_ast_check.h
**
**      Copyright (C) 2021-2024  Paul J. Lucas
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

#ifndef cdecl_c_ast_check_H
#define cdecl_c_ast_check_H

/**
 * @file
 * Declares functions for checking an AST for semantic errors.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "types.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdbool.h>

/// @endcond

/**
 * @ingroup ast-functions-group
 * @{
 */

////////// extern functions ///////////////////////////////////////////////////

/**
 * Checks an entire AST for semantic errors and warnings.
 *
 * @param ast The AST to check.
 * @return Returns `true` only if all checks passed.
 *
 * @sa c_ast_list_check()
 * @sa c_ast_check_typedef()
 */
NODISCARD
bool c_ast_check( c_ast_t const *ast );

/**
 * Performs additional checks on an AST for a type.
 *
 * @param type_ast The AST of a type to check.
 * @return Returns `true` only if all checks passed.
 *
 * @note It's presumed that c_ast_check() was called before calling this
 * function.
 *
 * @sa c_ast_check()
 */
NODISCARD
bool c_ast_check_typedef( c_ast_t const *type_ast );

/**
 * Checks a list of AST nodes that are part of the _same_ declaration for
 * semantic errors and warnings, for example:
 *
 *      int *p, *f(char);
 *
 * @param ast_list The list of AST nodes to check.
 * @return Returns `true` only if all checks passed.
 *
 * @sa c_ast_check()
 */
NODISCARD
bool c_ast_list_check( c_ast_list_t const *ast_list );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_c_ast_check_H */
/* vim:set et sw=2 ts=2: */
