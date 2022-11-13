/*
**      cdecl -- C gibberish translator
**      src/cdecl_parser.h
**
**      Copyright (C) 2021-2022  Paul J. Lucas
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

#ifndef cdecl_parser_H
#define cdecl_parser_H

/**
 * @file
 * Wrapper around the Bison-generated parser.h to add `#include` guards as well
 * as add declarations for Bison types and functions.
 */

// local
#include "parser.h"

// standard
#include <attribute.h>

///////////////////////////////////////////////////////////////////////////////

typedef enum yytokentype yytokentype;

/**
 * Cleans up global parser data at program termination.
 */
void parser_cleanup( void );

/**
 * Bison: parse input.
 *
 * @return Returns 0 only if parsing was successful.
 */
NODISCARD
int yyparse( void );

///////////////////////////////////////////////////////////////////////////////

#endif /* cdecl_parser_H */
/* vim:set et sw=2 ts=2: */
