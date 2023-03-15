/*
**      cdecl -- C gibberish translator
**      src/english.c
**
**      Copyright (C) 2017-2023  Paul J. Lucas
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
 * Defines functions for printing in pseudo-English.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "english.h"
#include "c_ast.h"
#include "c_ast_util.h"
#include "c_operator.h"
#include "c_typedef.h"
#include "literals.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>

/// @endcond

/**
 * @addtogroup printing-english-group
 * @{
 */

// local functions
NODISCARD
static bool c_ast_visitor_english( c_ast_t*, user_data_t );

static void c_type_print_not_base( c_type_t const*, FILE* );

////////// inline functions ///////////////////////////////////////////////////

/**
 * Convenience function for calling c_ast_visit() to print \a ast as a
 * declaration in pseudo-English.
 *
 * @param ast The AST to print.
 * @param eout The `FILE` to print to.
 */
static inline void c_ast_visit_english( c_ast_t const *ast, FILE *eout ) {
  c_ast_t *const nonconst_ast = CONST_CAST( c_ast_t*, ast );
  user_data_t const data = { .p = eout };
  c_ast_visit( nonconst_ast, C_VISIT_DOWN, c_ast_visitor_english, data );
}

////////// local functions ////////////////////////////////////////////////////

/**
 * Helper function for c_ast_visitor_english() that prints a bit-field width,
 * if any.
 *
 * @param ast The AST to print the bit-field width of.
 * @param eout The `FILE` to emit to.
 */
static void c_ast_bit_width_english( c_ast_t const *ast, FILE *eout ) {
  assert( ast != NULL );
  assert( is_1_bit_only_in_set( ast->kind, K_ANY_BIT_FIELD ) );
  assert( eout != NULL );

  if ( ast->bit_field.bit_width > 0 )
    FPRINTF( eout, " width %u bits", ast->bit_field.bit_width );
}

/**
 * Helper function for c_ast_visitor_english() that prints a function-like
 * AST's parameters, if any.
 *
 * @param ast The AST to print the parameters of.
 * @param eout The `FILE` to emit to.
 */
static void c_ast_func_params_english( c_ast_t const *ast, FILE *eout ) {
  assert( ast != NULL );
  assert( is_1_bit_only_in_set( ast->kind, K_ANY_FUNCTION_LIKE ) );
  assert( eout != NULL );

  FPUTC( '(', eout );

  bool comma = false;
  FOREACH_AST_FUNC_PARAM( param, ast ) {
    fprint_sep( eout, ", ", &comma );

    c_ast_t const *const param_ast = c_param_ast( param );
    c_sname_t const *const sname = c_ast_find_name( param_ast, C_VISIT_DOWN );
    if ( sname != NULL ) {
      //
      // For all kinds except K_NAME, we have to print:
      //
      //      <name> as <english>
      //
      // For K_NAME, e.g.:
      //
      //      void f(x)                 // untyped K&R C function parameter
      //
      // there's no "as <english>" part.
      //
      c_sname_english( sname, eout );
      if ( param_ast->kind != K_NAME )
        FPUTS( " as ", eout );
    }
    else {
      //
      // If there's no name, it's an unnamed parameter, e.g.:
      //
      //      void f(int)
      //
      // so there's no "<name> as" part.
      //
    }

    c_ast_visit_english( param_ast, eout );
  } // for

  FPUTC( ')', eout );
}

/**
 * Helper function for c_ast_visitor_english() that prints a lambda AST's
 * captures, if any.
 *
 * @param ast The lambda AST to print the captures of.
 * @param eout The `FILE` to emit to.
 */
static void c_ast_lambda_captures_english( c_ast_t const *ast, FILE *eout ) {
  assert( ast != NULL );
  assert( ast->kind == K_LAMBDA );
  assert( eout != NULL );

  FPUTC( '[', eout );

  bool comma = false;
  FOREACH_AST_LAMBDA_CAPTURE( capture, ast ) {
    fprint_sep( eout, ", ", &comma );

    c_ast_t const *const capture_ast = c_capture_ast( capture );
    assert( capture_ast->kind == K_CAPTURE );

    switch ( capture_ast->capture.kind ) {
      case C_CAPTURE_COPY:
        assert( c_sname_empty( &capture_ast->sname ) );
        FPUTS( "copy by default", eout );
        break;
      case C_CAPTURE_REFERENCE:
        if ( c_sname_empty( &capture_ast->sname ) )
          FPUTS( "reference by default", eout );
        else
          FPUTS( "reference to ", eout );
        break;
      case C_CAPTURE_THIS:
        FPUTS( L_this, eout );
        break;
      case C_CAPTURE_STAR_THIS:
        FPUTS( "*this", eout );
        break;
      case C_CAPTURE_VARIABLE:
        break;
    } // switch

    c_sname_english( &capture_ast->sname, eout );
  } // for

  FPUTC( ']', eout );
}

/**
 * Prints the scoped name of \a AST in pseudo-English.
 *
 * @param ast The AST to print the name of.
 * @param eout The `FILE` to emit to.
 */
static void c_ast_name_english( c_ast_t const *ast, FILE *eout ) {
  assert( ast != NULL );
  assert( eout != NULL );

  c_sname_t const *const found_sname = c_ast_find_name( ast, C_VISIT_DOWN );
  char const *local_name, *scope_name = "";
  c_type_t const *scope_type = NULL;

  if ( ast->kind == K_OPERATOR ) {
    local_name = c_oper_token_c( ast->oper.oper_id );
    if ( found_sname != NULL ) {
      scope_name = c_sname_full_name( found_sname );
      scope_type = c_sname_local_type( found_sname );
    }
  } else {
    assert( found_sname != NULL );
    assert( !c_sname_empty( found_sname ) );
    local_name = c_sname_local_name( found_sname );
    scope_name = c_sname_scope_name( found_sname );
    scope_type = c_sname_scope_type( found_sname );
  }

  assert( local_name != NULL );
  FPRINTF( eout, "%s ", local_name );
  if ( scope_name[0] != '\0' ) {
    assert( !c_type_is_none( scope_type ) );
    FPRINTF( eout, "of %s %s ", c_type_name_english( scope_type ), scope_name );
  }
}

/**
 * Visitor function that prints \a ast as pseudo-English.
 *
 * @param ast The AST to print.
 * @param data A pointer to a `FILE` to emit to.
 * @return Always returns `false`.
 */
NODISCARD
static bool c_ast_visitor_english( c_ast_t *ast, user_data_t data ) {
  assert( ast != NULL );
  FILE *const eout = data.p;
  assert( eout != NULL );

  switch ( ast->kind ) {
    case K_ARRAY:
      c_type_print_not_base( &ast->type, eout );
      if ( ast->array.size == C_ARRAY_SIZE_VARIABLE )
        FPUTS( "variable length ", eout );
      FPUTS( "array ", eout );
      fputs_sp( c_tid_name_english( ast->array.stids ), eout );
      if ( ast->array.size >= 0 )
        FPRINTF( eout, PRId_C_ARRAY_SIZE_T " ", ast->array.size );
      FPUTS( "of ", eout );
      break;

    case K_APPLE_BLOCK:
    case K_CONSTRUCTOR:
    case K_DESTRUCTOR:
    case K_FUNCTION:
    case K_OPERATOR:
    case K_USER_DEF_LITERAL:
      c_type_print_not_base( &ast->type, eout );
      switch ( ast->kind ) {
        case K_FUNCTION:
          if ( c_tid_is_any( ast->type.stids, TS_MEMBER_FUNC_ONLY ) )
            FPUTS( "member ", eout );
          break;
        case K_OPERATOR: {
          unsigned const overload_flags = c_ast_oper_overload( ast );
          char const *const op_literal =
            overload_flags == C_OP_MEMBER     ? "member "     :
            overload_flags == C_OP_NON_MEMBER ? "non-member " :
            "";
          FPUTS( op_literal, eout );
          break;
        }
        default:
          /* suppress warning */;
      } // switch

      FPUTS( c_kind_name( ast->kind ), eout );
      if ( c_ast_params_count( ast ) > 0 ) {
        FPUTC( ' ', eout );
        c_ast_func_params_english( ast, eout );
      }
      if ( ast->func.ret_ast != NULL )
        FPUTS( " returning ", eout );
      break;

    case K_BUILTIN:
      FPUTS( c_type_name_english( &ast->type ), eout );
      if ( ast->builtin.BitInt.width > 0 )
        FPRINTF( eout, " width %u bits", ast->builtin.BitInt.width );
      c_ast_bit_width_english( ast, eout );
      break;

    case K_CAPTURE:
      // A K_CAPTURE can occur only in a lambda capture list, not at the top-
      // level, and captures are never visited.
      UNEXPECTED_INT_VALUE( ast->kind );

    case K_CAST:
      if ( ast->cast.kind != C_CAST_C )
        FPRINTF( eout, "%s ", c_cast_english( ast->cast.kind ) );
      FPUTS( L_cast, eout );
      if ( !c_sname_empty( &ast->sname ) ) {
        FPUTC( ' ', eout );
        c_sname_english( &ast->sname, eout );
      }
      FPUTS( " into ", eout );
      break;

    case K_CLASS_STRUCT_UNION:
      FPRINTF( eout, "%s ", c_type_name_english( &ast->type ) );
      c_sname_english( &ast->csu.csu_sname, eout );
      break;

    case K_ENUM:
      FPRINTF( eout, "%s ", c_type_name_english( &ast->type ) );
      c_sname_english( &ast->enum_.enum_sname, eout );
      if ( ast->enum_.of_ast != NULL )
        FPUTS( " of type ", eout );
      else
        c_ast_bit_width_english( ast, eout );
      break;

    case K_LAMBDA:
      if ( !c_type_is_none( &ast->type ) )
        FPRINTF( eout, "%s ", c_type_name_english( &ast->type ) );
      FPUTS( L_lambda, eout );
      if ( c_ast_captures_count( ast ) > 0 ) {
        FPUTS( " capturing ", eout );
        c_ast_lambda_captures_english( ast, eout );
      }
      if ( c_ast_params_count( ast ) > 0 ) {
        FPUTC( ' ', eout );
        c_ast_func_params_english( ast, eout );
      }
      if ( ast->lambda.ret_ast != NULL )
        FPUTS( " returning ", eout );
      break;

    case K_NAME:
      // A K_NAME can occur only as an untyped K&R C function parameter.  The
      // name was printed in c_ast_func_params_english().
      break;

    case K_POINTER:
    case K_REFERENCE:
    case K_RVALUE_REFERENCE:
      c_type_print_not_base( &ast->type, eout );
      FPRINTF( eout, "%s to ", c_kind_name( ast->kind ) );
      break;

    case K_POINTER_TO_MEMBER:
      c_type_print_not_base( &ast->type, eout );
      FPRINTF( eout, "%s of ", c_kind_name( ast->kind ) );
      fputs_sp( c_tid_name_english( ast->type.btids ), eout );
      c_sname_english( &ast->ptr_mbr.class_sname, eout );
      FPUTC( ' ', eout );
      break;

    case K_TYPEDEF:
      if ( !c_type_equiv( &ast->type, &C_TYPE_LIT_B( TB_TYPEDEF ) ) )
        FPRINTF( eout, "%s ", c_type_name_english( &ast->type ) );
      c_sname_english( &ast->tdef.for_ast->sname, eout );
      c_ast_bit_width_english( ast, eout );
      break;

    case K_USER_DEF_CONVERSION:
      fputs_sp( c_type_name_english( &ast->type ), eout );
      FPUTS( c_kind_name( ast->kind ), eout );
      if ( !c_sname_empty( &ast->sname ) ) {
        FPRINTF( eout,
          " of %s ", c_type_name_english( c_sname_local_type( &ast->sname ) )
        );
        c_sname_english( &ast->sname, eout );
      }
      FPUTS( " returning ", eout );
      break;

    case K_VARIADIC:
      FPUTS( c_kind_name( ast->kind ), eout );
      break;

    CASE_K_PLACEHOLDER;
  } // switch

  return /*stop=*/false;
}

/**
 * Helper function for c_sname_english() that prints the scopes' types and
 * names in inner-to-outer order except for the inner-most scope.  For example,
 * `S::T::x` is printed as "of scope T of scope S."
 *
 * @param scope A pointer to the outermost scope.
 * @param eout The `FILE` to emit to.
 */
static void c_scope_english( c_scope_t const *scope, FILE *eout ) {
  assert( scope != NULL );
  assert( eout != NULL );

  if ( scope->next != NULL ) {
    c_scope_english( scope->next, eout );
    c_scope_data_t const *const data = c_scope_data( scope );
    FPRINTF( eout,
      " of %s %s", c_type_name_english( &data->type ), data->name
    );
  }
}

/**
 * Prints the non-base (attribute(s), storage class, qualifier(s), etc.) parts
 * of \a type, if any.
 *
 * @param type The type to perhaps print.
 * @param eout The `FILE` to emit to.
 */
static void c_type_print_not_base( c_type_t const *type, FILE *eout ) {
  assert( type != NULL );
  assert( eout != NULL );

  c_type_t const nobase_type = { TB_NONE, type->stids, type->atids };
  fputs_sp( c_type_name_english( &nobase_type ), eout );
}

////////// extern functions ///////////////////////////////////////////////////

void c_ast_english( c_ast_t const *ast, FILE *eout ) {
  assert( ast != NULL );
  assert( eout != NULL );

  if ( ast->kind != K_CAST ) {
    FPUTS( "declare ", eout );
    switch ( ast->kind ) {
      case K_LAMBDA:
      case K_USER_DEF_CONVERSION:
        break;                          // these don't have names
      default:
        c_ast_name_english( ast, eout );
        FPUTS( "as ", eout );
    } // switch
  }

  c_ast_visit_english( ast, eout );

  switch ( ast->align.kind ) {
    case C_ALIGNAS_NONE:
      break;
    case C_ALIGNAS_EXPR:
      if ( ast->align.expr > 0 )
        FPRINTF( eout, " aligned as %u bytes", ast->align.expr );
      break;
    case C_ALIGNAS_TYPE:
      FPUTS( " aligned as ", eout );
      c_ast_visit_english( ast->align.type_ast, eout );
      break;
  } // switch

  FPUTC( '\n', eout );
}

char const* c_cast_english( c_cast_kind_t kind ) {
  switch ( kind ) {
    case C_CAST_C           : return "C";
    case C_CAST_CONST       : return L_const;
    case C_CAST_DYNAMIC     : return L_dynamic;
    case C_CAST_REINTERPRET : return L_reinterpret;
    case C_CAST_STATIC      : return L_static;
  } // switch
  UNEXPECTED_INT_VALUE( kind );
}

void c_sname_english( c_sname_t const *sname, FILE *eout ) {
  assert( sname != NULL );
  assert( eout != NULL );

  if ( !c_sname_empty( sname ) ) {
    FPUTS( c_sname_local_name( sname ), eout );
    c_scope_english( sname->head, eout );
  }
}

void c_typedef_english( c_typedef_t const *tdef, FILE *eout ) {
  assert( tdef != NULL );
  assert( tdef->ast != NULL );
  assert( eout != NULL );

  FPUTS( "define ", eout );
  c_sname_english( &tdef->ast->sname, eout );
  FPUTS( " as ", eout );
  c_ast_visit_english( tdef->ast, eout );
}

///////////////////////////////////////////////////////////////////////////////

/** @} */

/* vim:set et sw=2 ts=2: */
