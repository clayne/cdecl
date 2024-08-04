/*
**      cdecl -- C gibberish translator
**      src/options.h
**
**      Copyright (C) 2017-2024  Paul J. Lucas, et al.
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

#ifndef cdecl_options_H
#define cdecl_options_H

/**
 * @file
 * Declares global variables and functions for **cdecl** options.
 *
 * @sa cli_options.h
 * @sa set_options.h
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "c_kind.h"
#include "color.h"
#include "types.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdbool.h>

/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * @defgroup cdecl-options-group Cdecl Options
 * Global variables and functions for **cdecl** options.
 *
 * @sa \ref cli-options-group
 * @sa \ref set-options-group
 *
 * @{
 */

// extern option variables
extern bool         opt_alt_tokens;     ///< Print alternative tokens?

#ifdef ENABLE_BISON_DEBUG
/// Print Bison debug output?
///
/// @note This is an alias for \ref yydebug defined for consistency with the
/// naming of **cdecl**'s other options.
#define             opt_bison_debug     yydebug
#endif /* ENABLE_BISON_DEBUG */

extern cdecl_debug_t opt_cdecl_debug;   ///< Print JSON5 debug output?

extern color_when_t opt_color_when;     ///< When to print color.
extern char const  *opt_config_path;    ///< Configuration file path.
extern bool         opt_east_const;     ///< Print in "east const" form?
extern bool         opt_echo_commands;  ///< Echo commands?
extern bool         opt_english_types;  ///< Print types in English, not C/C++.

/// Explicit `enum` | `class` | `struct` | `union`?
extern c_tid_t      opt_explicit_ecsu_btids;

extern char const  *opt_file;           ///< Read from this file.

#ifdef ENABLE_FLEX_DEBUG
/// Print Flex debug output?
///
/// @note This is an alias for \ref yy_flex_debug defined for consistency with
/// the naming of **cdecl**'s other options.
#define             opt_flex_debug      yy_flex_debug
#endif /* ENABLE_FLEX_DEBUG */

extern c_graph_t    opt_graph;          ///< Di/Trigraph mode.
extern bool         opt_infer_command;  ///< Infer command if none given?
extern c_lang_id_t  opt_lang_id;        ///< Current language.
extern unsigned     opt_lineno;         ///< Add to all line numbers.

/// Allow unknown names and keywords in other languages to be types?
extern bool         opt_permissive_types;

extern bool         opt_prompt;         ///< Print the prompt?
extern bool         opt_read_config;    ///< Read configuration file?
extern bool         opt_semicolon;      ///< Print `;` at end of gibberish?
extern bool         opt_trailing_ret;   ///< Print trailing return type?
extern bool         opt_typedefs;       ///< Load C/C++ standard `typedef`s?
extern bool         opt_using;          ///< Print `using` in C++11 and later?

/// Kinds to print `*` and `&` "west" of the space.
extern c_ast_kind_t opt_west_decl_kinds;

/// What `*` expands into for `set debug=*`.
extern char const   OPT_CDECL_DEBUG_ALL[];

/// What `*` expands into for `set escu=*`.
extern char const   OPT_ECSU_ALL[];

/// What `*` expands into for `set west-decl=*`.
extern char const   OPT_WEST_DECL_ALL[];

// other extern variables
#ifdef ENABLE_FLEX_DEBUG
/// Flex variable for debugging; use #opt_flex_debug instead.
extern int          yy_flex_debug;
#endif /* ENABLE_FLEX_DEBUG */
#ifdef ENABLE_BISON_DEBUG
/// Bison variable for debugging; use #opt_bison_debug instead.
extern int          yydebug;
#endif /* ENABLE_BISON_DEBUG */

////////// extern functions ///////////////////////////////////////////////////

/**
 * Gets the string representation of the **cdecl** debug option.
 *
 * @return Returns said representation.
 *
 * @warning The pointer returned is to a static buffer.  Changing the value of
 * \ref opt_explicit_ecsu_btids then calling this function again will change
 * the value of the buffer.
 *
 * @sa parse_cdecl_debug()
 */
NODISCARD
char const* cdecl_debug_str( void );

/**
 * Gets the string representation of the explicit `enum`, `class`, `struct`,
 * `union` option.
 *
 * @return Returns said representation.
 *
 * @warning The pointer returned is to a static buffer.  Changing the value of
 * \ref opt_explicit_ecsu_btids then calling this function again will change
 * the value of the buffer.
 *
 * @sa parse_explicit_ecsu()
 */
NODISCARD
char const* explicit_ecsu_str( void );

/**
 * Gets the string representation of the explicit integer option.
 *
 * @return Returns said representation.
 *
 * @warning The pointer returned is to a static buffer.  Changing the value of
 * the option via parse_explicit_int() then calling this function again will
 * change the value of the buffer.
 *
 * @sa is_explicit_int()
 * @sa parse_explicit_int()
 */
NODISCARD
char const* explicit_int_str( void );

/**
 * Checks whether \a btids shall have `int` be printed explicitly for it.
 *
 * @param btids The integer type to check.
 * @return Returns `true` only if the type given by \a btid shall have `int`
 * printed explicitly.
 *
 * @sa parse_explicit_int()
 * @sa explicit_int_str()
 */
NODISCARD
bool is_explicit_int( c_tid_t btids );

/**
 * Sets the current language option and the corresponding prompt.
 *
 * @param lang_id The language to set.  _Exactly one_ language _must_ be set.
 */
void lang_set( c_lang_id_t lang_id );

/**
 * Parses the **cdecl** debug option.
 *
 * @param debug_format
 * @parblock
 * The null-terminated **cdecl** debut option format
 * string (case insensitive) to parse.  Valid formats are:
 *
 * Format | Meaning
 * -------|---------------------
 * `u`    | Include `unique_id`.
 *
 * Alternatively, `*` may be given to mean "all", NULL may be given to mean set
 * with no options, or `-` or the empty string may be given to mean "none."
 * @endparblock
 * @return Returns `true` only if \a debug_format was parsed successfully.
 *
 * @sa cdecl_debug_str()
 */
NODISCARD
bool parse_cdecl_debug( char const *debug_format );

/**
 * Parses the explicit `enum`, `class`, `struct`, `union` option.
 *
 * @param ecsu_format
 * @parblock
 * The null-terminated explicit `enum`, `class`, `struct`, `union` format
 * string (case insensitive) to parse.  Valid formats are:
 *
 * Format | Meaning
 * -------|--------
 * `e`    | `enum`
 * `c`    | `class`
 * `s`    | `struct`
 * `u`    | `union`
 *
 * Multiple formats may be given, one immediately after the other, e.g., `su`
 * means `struct` and `union`.  Alternatively, `*` may be given to mean "all"
 * or either the empty string or `-` may be given to mean "none."
 * @endparblock
 * @return Returns `true` only if \a ecsu_format was parsed successfully.
 *
 * @sa explicit_ecsu_str()
 */
NODISCARD
bool parse_explicit_ecsu( char const *ecsu_format );

/**
 * Parses the explicit `int` option.
 *
 * @param ei_format
 * @parblock
 * The null-terminated explicit `int` format string (case insensitive) to
 * parse.  Valid formats are:
 *
 * Format                    | Meaning
 * --------------------------|----------------------------
 *    `i`                    | All signed integer types.
 * `u`                       | All unsigned integer types.
 * [`u`]{`i`\|`s`\|`l`[`l`]} | Possibly `unsigned` `int`, `short`, `long`, or `long long`.
 *
 * Multiple formats may be given, one immediately after the other, e.g., `usl`
 * means `unsigned short` and `long`.  Parsing is greedy so commas may be used
 * to separate formats.  For example, `ulll` is parsed as `unsigned long long`
 * and `long` whereas `ul,ll` is parsed as `unsigned long` and `long long`.  If
 * invalid, an error message is printed to standard error.  Alternatively,
 * `*` may be given to mean "all" or either the empty string or `-` may be
 * given to mean "none."
 * @endparblock
 * @return Returns `true` only if \a ei_format was parsed successfully.
 *
 * @sa is_explicit_int()
 * @sa explicit_int_str()
 */
NODISCARD
bool parse_explicit_int( char const *ei_format );

/**
 * Parses the `west-decl` option.
 *
 * @param wd_format
 * @parblock
 * The null-terminated west pointer format string to parse.  Valid formats are:
 *
 * Format | Meaning
 * -------|-----------------------------------
 * `b`    | Apple block return type.
 * `f`    | Function (and pointer to function) return type.
 * `l`    | User-defined literal return type.
 * `o`    | Operator return type.
 * `r`    | All return types (same as `bflo`).
 * `t`    | Non-return types.
 *
 * Multiple formats may be given, one immediately after the other.
 * Alternatively, `*` may be given to mean "all" or either the empty string or
 * `-` may be given to mean "none."
 * @endparblock
 * @return Returns `true` only if \a wd_format was parsed successfully.
 *
 * @sa west_decl_str()
 */
NODISCARD
bool parse_west_decl( char const *wd_format );

/**
 * Gets the string representation of the west pointer option.
 *
 * @return Returns said representation.
 *
 * @warning The pointer returned is to a static buffer.  Changing the value of
 * \ref opt_west_decl_kinds then calling this function again will change the
 * value of the buffer.
 *
 * @sa parse_west_decl()
 */
NODISCARD
char const* west_decl_str( void );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_options_H */
/* vim:set et sw=2 ts=2: */
