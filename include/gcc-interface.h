/* Interface between GCC C FE and GDB

   Copyright (C) 2014 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GDB_GCC_INTERFACE
#define GDB_GCC_INTERFACE

/* This header defines the interface to the GCC API.  It must be both
   valid C and valid C++, because it is included by both programs.  */

/* One bit of GCC internals leaks through here.  */

union tree_node;
typedef union tree_node *gcc_type;
typedef union tree_node *gcc_decl;

/* An address in the inferior.  */

typedef unsigned long long gcc_address;

struct gcc_base_context;
struct gcc_c_context;

/* The operations defined by the GCC base API.  This is the vtable for
   the real context structure which is passed around.
   
   The "base" API is concerned with basics shared by all compiler
   front ends: setting command-line arguments, the file names, etc.

   Front-end-specific interfaces inherit from this one.  */

struct gcc_base_vtable
{
  /* The actual version implemented in this interface.  This field can
     be relied on not to move, so users can always check it if they
     desire.  */

  unsigned int version;

  /* Set the compiler's command-line options for the next compliation.
     The arguments are copied by GCC.  ARGV need not be
     NULL-terminated.  The arguments must be set separately for each
     compilation; that is, after a compile is requested, the
     previously-set arguments cannot be reused.  */

  void (*set_arguments) (struct gcc_base_context *self, int argc, char **argv);

  /* Set the file name of the program to compile.  The string is
     copied by the method implementation, but the caller must
     guarantee that the file exists through the compilation.  */

  void (*set_source_file) (struct gcc_base_context *self, const char *file);

  /* Set a callback to use for printing error messages.  DATUM is
     passed through to the callback unchanged.  */

  void (*set_print_callback) (struct gcc_base_context *self,
			      void (*print_function) (void *datum,
						      const char *message),
			      void *datum);

  /* Perform the compilation.  FILENAME is the name of the resulting
     object file.  VERBOSE can be set to cause GCC to print some
     information as it works.  Returns true on success, false on
     error.  */

  int /* bool */ (*compile) (struct gcc_base_context *self,
			     const char *filename,
			     int /* bool */ verbose);

  /* Destroy this object.  */

  void (*destroy) (struct gcc_base_context *self);
};

/* The GCC object.  */

struct gcc_base_context
{
  /* The virtual table.  */

  const struct gcc_base_vtable *ops;
};



/*
 * Definitions and declarations for the C front end.
 */

enum gcc_qualifiers
{
  GCC_QUALIFIER_CONST = 1,
  GCC_QUALIFIER_VOLATILE = 2,
  GCC_QUALIFIER_RESTRICT = 4
};

/* This enumerates the kinds of decls that GDB can create.  */

enum gcc_c_symbol_kind
{
  /* A function.  */

  GCC_C_SYMBOL_FUNCTION,

  /* A variable.  */

  GCC_C_SYMBOL_VARIABLE,

  /* A typedef.  */

  GCC_C_SYMBOL_TYPEDEF,

  /* A label.  */

  GCC_C_SYMBOL_LABEL
};

/* This enumerates the types of symbols that GCC might request from
   GDB.  */

enum gcc_c_oracle_request
{
  /* An ordinary symbol -- a variable, function, typedef, or enum
     constant.  */

  GCC_C_ORACLE_SYMBOL,

  /* A struct, union, or enum tag.  */

  GCC_C_ORACLE_TAG,

  /* A label.  */

  GCC_C_ORACLE_LABEL
};

/* The type of the function called by GCC to ask GDB for a symbol's
   definition.  DATUM is an arbitrary value supplied when the oracle
   function is registered.  CONTEXT is the GCC context in which the
   request is being made.  REQUEST specifies what sort of symbol is
   being requested, and IDENTIFIER is the name of the symbol.  */

typedef void gcc_c_oracle_function (void *datum,
				    struct gcc_c_context *context,
				    enum gcc_c_oracle_request request,
				    const char *identifier);

/* The type of the function called by GCC to ask GDB for a symbol's
   address.  This should return 0 if the address is not known.  */

typedef gcc_address gcc_c_symbol_address_function (void *datum,
						   struct gcc_c_context *ctxt,
						   const char *identifier);

/* The vtable used by the C front end.  */

struct gcc_c_fe_vtable
{
  /* Set the callbacks for this context.

     The binding oracle is called whenever the C parser needs to look
     up a symbol.  This gives the caller a chance to lazily
     instantiate symbols using other parts of the gcc_c_fe_interface
     API.

     The address oracle is called whenever the C parser needs to look
     up a symbol.  This is only called for symbols not provided by the
     symbol oracle -- that is, just built-in functions where GCC
     provides the declaration.

     DATUM is an arbitrary piece of data that is passed back verbatim
     to the callbakcs in requests.  */

  void (*set_callbacks) (struct gcc_c_context *self,
			 gcc_c_oracle_function *binding_oracle,
			 gcc_c_symbol_address_function *address_oracle,
			 void *datum);

  /* Create a new "decl" in GCC.  A decl is a declaration, basically a
     kind of symbol.

     NAME is the name of the new symbol.  SYM_KIND is the kind of
     symbol being requested.  SYM_TYPE is the new symbol's C type;
     except for labels, where this is not meaningful and should be
     NULL.  If SUBSTITUTION_NAME is not NULL, then a reference to this
     decl in the source will later be substituted with a dereference
     of a variable of the given name.  Otherwise, for symbols having
     an address (e.g., functions), ADDRESS is the address.  FILENAME
     and LINE_NUMBER refer to the symbol's source location.  If this
     is not known, FILENAME can be NULL and LINE_NUMBER can be 0.
     This function returns the new decl.  */

  gcc_decl (*build_decl) (struct gcc_c_context *self,
			  const char *name,
			  enum gcc_c_symbol_kind sym_kind,
			  gcc_type sym_type,
			  const char *substitution_name,
			  gcc_address address,
			  const char *filename,
			  unsigned int line_number);

  /* Insert a GCC decl into the symbol table.  DECL is the decl to
     insert.  IS_GLOBAL is true if this is an outermost binding, and
     false if it is a possibly-shadowing binding.  */

  void (*bind) (struct gcc_c_context *self, gcc_decl decl,
		int /* bool */ is_global);

  /* Insert a tagged type into the symbol table.  NAME is the tag name
     of the type and TAGGED_TYPE is the type itself.  TAGGED_TYPE must
     be either a struct, union, or enum type, as these are the only
     types that have tags.  */

  void (*tagbind) (struct gcc_c_context *self, const char *name,
		   gcc_type tagged_type);

  /* Return the type of a pointer to a given base type.  */

  gcc_type (*build_pointer_type) (struct gcc_c_context *self,
				  gcc_type base_type);

  /* Create a new 'struct' type.  Initially it has no fields.  */

  gcc_type (*build_record_type) (struct gcc_c_context *self);

  /* Create a new 'union' type.  Initially it has no fields.  */

  gcc_type (*build_union_type) (struct gcc_c_context *self);

  /* Add a field to a struct or union type.  FIELD_NAME is the field's
     name.  FIELD_TYPE is the type of the field.  BITSIZE and BITPOS
     indicate where in the struct the field occurs.  */

  void (*build_add_field) (struct gcc_c_context *self,
			   gcc_type record_or_union_type,
			   const char *field_name,
			   gcc_type field_type,
			   unsigned long bitsize,
			   unsigned long bitpos);

  /* After all the fields have been added to a struct or union, the
     struct or union type must be "finished".  This does some final
     cleanups in GCC.  */

  void (*finish_record_or_union) (struct gcc_c_context *self,
				  gcc_type record_or_union_type,
				  unsigned long size_in_bytes);

  /* Create a new 'enum' type.  The new type initially has no
     associated constants.  */

  gcc_type (*build_enum_type) (struct gcc_c_context *self,
			       gcc_type underlying_int_type);

  /* Add a new constant to an enum type.  NAME is the constant's
     name and VALUE is its value.  */

  void (*build_add_enum_constant) (struct gcc_c_context *self,
				   gcc_type enum_type,
				   const char *name,
				   unsigned long value);

  /* After all the constants have been added to an enum, the type must
     be "finished".  This does some final cleanups in GCC.  */

  void (*finish_enum_type) (struct gcc_c_context *self, gcc_type enum_type);

  /* Create a new function type.  RETURN_TYPE is the type returned by
     the function, and ARGUMENT_TYPES is a vector, of length NARGS, of
     the argument types.  IS_VARARGS is true if the function is
     varargs.  */

  gcc_type (*build_function_type) (struct gcc_c_context *self,
				   gcc_type return_type,
				   int nargs,
				   gcc_type *argument_types,
				   int /* bool */ is_varargs);

  /* Return an integer type with the given properties.  */

  gcc_type (*int_type) (struct gcc_c_context *self,
			int /* bool */ is_unsigned,
			unsigned long size_in_bytes);

  /* Return a floating point type with the given properties.  */

  gcc_type (*float_type) (struct gcc_c_context *self,
			  unsigned long size_in_bytes);

  /* Return the 'void' type.  */

  gcc_type (*void_type) (struct gcc_c_context *self);

  /* Return the 'bool' type.  */

  gcc_type (*bool_type) (struct gcc_c_context *self);

  /* Create a new array type.  If NUM_ELEMENTS is -1, then the array
     is assumed to have an unknown length.  */

  gcc_type (*build_array_type) (struct gcc_c_context *self,
				gcc_type element_type, int num_elements);

  /* Return a qualified variant of a given base type.  QUALIFIERS says
     which qualifiers to use; it is composed of or'd together
     constants from 'enum gcc_qualifiers'.  */

  gcc_type (*build_qualified_type) (struct gcc_c_context *self,
				    gcc_type unqualified_type,
				    int /* enum gcc_qualifiers */ qualifiers);

  /* Build a complex type given its element type.  */

  gcc_type (*build_complex_type) (struct gcc_c_context *self,
				  gcc_type element_type);

  /* Build a vector type given its element type and number of
     elements.  */

  gcc_type (*build_vector_type) (struct gcc_c_context *self,
				 gcc_type element_type,
				 int num_elements);

  /* Emit an error and return an error type object.  */

  gcc_type (*error) (struct gcc_c_context *self,
		     const char *message);
};

/* The C front end object.  */

struct gcc_c_context
{
  /* Base class.  */

  struct gcc_base_context base;

  /* Our vtable.  This is a separate field because this is simpler
     than implementing a vtable inheritance scheme in C.  */

  const struct gcc_c_fe_vtable *c_ops;
};

/* Currently only a single version is defined.  */

#define GCC_C_FE_VERSION 0

/* The name of the .so that the compiler builds.  We dlopen this
   later.  */

#define GCC_C_FE_LIBCC libcc1.so

/* The compiler exports a single initialization function.  This macro
   holds its name as a symbol.  */

#define GCC_C_FE_CONTEXT gcc_c_fe_context

/* The type of the initialization function.  The caller passes in the
   desired version.  If the request can be satisfied, a compatible
   gcc_context object will be returned.  Otherwise, the function
   returns NULL.  */

typedef struct gcc_c_context *gcc_c_fe_context_function (unsigned int);

/* The name of the dummy wrapper function generated by gdb.  */

#define GCC_C_FE_WRAPPER_FUNCTION "_gdb_expr"

#endif /* GDB_GCC_INTERFACE */
