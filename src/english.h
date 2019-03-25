/*
**      cdecl -- C gibberish translator
**      src/english.h
**
**      Copyright (C) 2017-2019  Paul J. Lucas, et al.
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

#ifndef cdecl_english_H
#define cdecl_english_H

/**
 * @file
 * Defines functions for printing an AST in pseudo-English.
 */

// local
#include "cdecl.h"                      /* must go first */
#include "typedefs.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdio.h>                      /* for FILE */

/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * Prints \a ast in pseudo-English.
 *
 * @param ast The `c_ast` to print.
 * @param eout The `FILE` to print to.
 */
void c_ast_english( c_ast_t const *ast, FILE *eout );

/**
 * Prints \a sname in pseudo-English.
 *
 * @param sname The name to print.
 * @param eout The `FILE` to print to.
 */
void c_sname_english( c_sname_t const *sname, FILE *eout );

///////////////////////////////////////////////////////////////////////////////

#endif /* cdecl_english_H */
/* vim:set et sw=2 ts=2: */
