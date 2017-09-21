/*
**      cdecl -- C gibberish translator
**      src/common.h
**
**      Copyright (C) 2017  Paul J. Lucas, et al.
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

#ifndef cdecl_common_H
#define cdecl_common_H

/**
 * @file
 * Declares constants, macros, types, and global variables, that are common to
 * several files that don't really fit anywhere else.
 */

// local
#include "config.h"                     /* must go first */

// standard
#include <stdbool.h>
#include <stddef.h>                     /* for size_t */

///////////////////////////////////////////////////////////////////////////////

#define CONF_FILE_NAME            "." PACKAGE "rc"
#define CPPDECL                   "c++decl"
#define DEBUG_INDENT              2     /* spaces per debug indent level */

/**
 * Mode of operation.
 */
enum c_mode {
  MODE_ENGLISH,
  MODE_GIBBERISH
};
typedef enum c_mode c_mode_t;

/**
 * The source location used by bison.
 */
typedef struct {
  size_t first_line;
  size_t first_column;
  size_t last_line;
  size_t last_column;
} YYLTYPE;

#define YYLTYPE_IS_DECLARED       1
#define YYLTYPE_IS_TRIVIAL        1

typedef YYLTYPE c_loc_t;

// extern variables
extern c_mode_t     c_mode;             // parsing english or gibberish?
extern char const  *command_line;       // command from command line, if any
extern size_t       command_line_len;   // length of command_line
extern bool         is_input_a_tty;     // is our input from a tty?
extern char const  *me;                 // program name

///////////////////////////////////////////////////////////////////////////////

#endif /* cdecl_common_H */
/* vim:set et sw=2 ts=2: */
