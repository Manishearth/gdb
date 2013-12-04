/* Private partial symbol table definitions.

   Copyright (C) 2009-2014 Free Software Foundation, Inc.

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

#ifndef PSYMPRIV_H
#define PSYMPRIV_H

#include "psymtab.h"

struct psymbol_allocation_list;

/* A partial_symbol records the name, domain, and address class of
   symbols whose types we have not parsed yet.  For functions, it also
   contains their memory address, so we can find them from a PC value.
   Each partial_symbol sits in a partial_symtab, all of which are chained
   on a  partial symtab list and which points to the corresponding 
   normal symtab once the partial_symtab has been referenced.  */

/* This structure is space critical.  See space comments at the top of
   symtab.h.  */

struct partial_symbol
{

  /* The general symbol info required for all types of symbols.  */

  struct general_symbol_info pginfo;

  /* Name space code.  */

  ENUM_BITFIELD(domain_enum_tag) domain : 6;

  /* Address class (for info_symbols).  Note that we don't allow
     synthetic "aclass" values here at present, simply because there's
     no need.  */

  ENUM_BITFIELD(address_class) aclass : 6;

};

#define PSYMBOL_VALUE(symbol)		(symbol)->pginfo.value.ivalue
#define SET_PSYMBOL_VALUE_ADDRESS(symbol, addr) \
  (((symbol)->pginfo.value.address) = (addr))
#define PSYMBOL_VALUE_RAW_ADDRESS(symbol) ((symbol)->pginfo.value.address + 0)
#define PSYMBOL_VALUE_ADDRESS(objfile, symbol)	\
  ((symbol)->pginfo.value.address \
   + (0 * (ANOFFSET ((objfile)->section_offsets, ((symbol)->pginfo.section)))))
#define PSYMBOL_LANGUAGE(symbol)	(symbol)->pginfo.language
#define PSYMBOL_SECTION(symbol)		(symbol)->pginfo.section
#define PSYMBOL_OBJ_SECTION(objfile, symbol)			\
  (((symbol)->pginfo.section >= 0)				\
   ? (&(((objfile)->sections)[(symbol)->pginfo.section]))	\
   : NULL)

#define PSYMBOL_SET_LANGUAGE(symbol,language,obstack)	\
  (symbol_set_language (&(symbol)->pginfo, (language), (obstack)))
#define PSYMBOL_SET_NAMES(symbol,linkage_name,len,copy_name,objfile)	\
  symbol_set_names (&(symbol)->pginfo, linkage_name, len, copy_name, objfile)

#define PSYMBOL_LINKAGE_NAME(symbol)	(symbol)->pginfo.name
#define PSYMBOL_DEMANGLED_NAME(symbol) \
  (symbol_demangled_name (&(symbol)->pginfo))
#define PSYMBOL_SEARCH_NAME(symbol)					 \
   (symbol_search_name (&(symbol)->pginfo))

#define PSYMBOL_DOMAIN(psymbol)	(psymbol)->domain
#define PSYMBOL_CLASS(psymbol)		(psymbol)->aclass

#define PSYMBOL_MATCHES_SEARCH_NAME(symbol, name)			\
  (strcmp_iw (PSYMBOL_SEARCH_NAME (symbol), (name)) == 0)

/* A convenience enum to give names to some constants used when
   searching psymtabs.  This is internal to psymtab and should not be
   used elsewhere.  */

enum psymtab_search_status
  {
    PST_NOT_SEARCHED,
    PST_SEARCHED_AND_FOUND,
    PST_SEARCHED_AND_NOT_FOUND
  };

/* Each source file that has not been fully read in is represented by
   a partial_symtab.  This contains the information on where in the
   executable the debugging symbols for a specific file are, and a
   list of names of global symbols which are located in this file.
   They are all chained on partial symtab lists.

   Even after the source file has been read into a symtab, the
   partial_symtab remains around.  They are allocated on an obstack,
   objfile_obstack.  */

struct partial_symtab
{

  /* Chain of all existing partial symtabs.  */

  struct partial_symtab *next;

  /* Name of the source file which this partial_symtab defines,
     or if the psymtab is anonymous then a descriptive name for
     debugging purposes, or "".  It must not be NULL.  */

  const char *filename;

  /* Full path of the source file.  NULL if not known.  */

  char *fullname;

  /* Directory in which it was compiled, or NULL if we don't know.  */

  const char *dirname;

  /* Set of relocation offsets to apply to each section.  Offsets here
     are applied when expanding this psymtab to a full symtab.  This
     is typically NULL, which means no additional offsets need to be
     applied, but in some cases it's different.  See, e.g.,
     elfstab_offset_sections.  */

  struct section_offsets *section_offsets;

  /* Range of text addresses covered by this file; texthigh is the
     beginning of the next section.  Do not use if PSYMTABS_ADDRMAP_SUPPORTED
     is set.  Do not refer directly to these fields.  Instead, use the
     accessor macros.  The validity of these fields is determined by the
     textlow_valid and texthigh_valid fields; these are located later
     in this structure for better packing.  */

  CORE_ADDR textlow_;
  CORE_ADDR texthigh_;

  /* If NULL, this is an ordinary partial symbol table.

     If non-NULL, this holds a single includer of this partial symbol
     table, and this partial symbol table is a shared one.

     A shared psymtab is one that is referenced by multiple other
     psymtabs, and which conceptually has its contents directly
     included in those.

     Shared psymtabs have special semantics.  When a search finds a
     symbol in a shared table, we instead return one of the non-shared
     tables that include this one.

     A shared psymtabs can be referred to by other shared ones.

     The psymtabs that refer to a shared psymtab will list the shared
     psymtab in their 'dependencies' array.

     In DWARF terms, a shared psymtab is a DW_TAG_partial_unit; but
     of course using a name based on that would be too confusing, so
     "shared" was chosen instead.
     
     Only a single user is needed because, when expanding a shared
     psymtab, we only need to expand its "canonical" non-shared user.
     The choice of which one should be canonical is left to the
     debuginfo reader; it can be arbitrary.  */

  struct partial_symtab *user;

  /* Array of pointers to all of the partial_symtab's which this one
     depends on.  Since this array can only be set to previous or
     the current (?) psymtab, this dependency tree is guaranteed not
     to have any loops.  "depends on" means that symbols must be read
     for the dependencies before being read for this psymtab; this is
     for type references in stabs, where if foo.c includes foo.h, declarations
     in foo.h may use type numbers defined in foo.c.  For other debugging
     formats there may be no need to use dependencies.  */

  struct partial_symtab **dependencies;

  int number_of_dependencies;

  /* Global symbol list.  This list will be sorted after readin to
     improve access.  Binary search will be the usual method of
     finding a symbol within it.  globals_offset is an integer offset
     within global_psymbols[].  */

  int globals_offset;
  int n_global_syms;

  /* Static symbol list.  This list will *not* be sorted after readin;
     to find a symbol in it, exhaustive search must be used.  This is
     reasonable because searches through this list will eventually
     lead to either the read in of a files symbols for real (assumed
     to take a *lot* of time; check) or an error (and we don't care
     how long errors take).  This is an offset and size within
     static_psymbols[].  */

  int statics_offset;
  int n_static_syms;

  /* Non-zero if the symtab corresponding to this psymtab has been
     readin.  This is located here so that this structure packs better
     on 64-bit systems.  */

  unsigned char readin;

  /* True iff objfile->psymtabs_addrmap is properly populated for this
     partial_symtab.  For discontiguous overlapping psymtabs is the only usable
     info in PSYMTABS_ADDRMAP.  */

  unsigned char psymtabs_addrmap_supported;

  /* True if the name of this partial symtab is not a source file name.  */

  unsigned char anonymous;

  /* A flag that is temporarily used when searching psymtabs.  */

  ENUM_BITFIELD (psymtab_search_status) searched_flag : 2;

  /* Validity of the textlow_ and texthigh_ fields.  */

  unsigned int textlow_valid : 1;
  unsigned int texthigh_valid : 1;

  /* Pointer to symtab eventually allocated for this source file, 0 if
     !readin or if we haven't looked for the symtab after it was readin.  */

  struct symtab *symtab;

  /* Pointer to function which will read in the symtab corresponding to
     this psymtab.  */

  void (*read_symtab) (struct partial_symtab *, struct objfile *);

  /* Information that lets read_symtab() locate the part of the symbol table
     that this psymtab corresponds to.  This information is private to the
     format-dependent symbol reading routines.  For further detail examine
     the various symbol reading modules.  */

  void *read_symtab_private;
};

/* Compute a section offset given an objfile, a section_offsets field
   from a partial symtab, and an index.  */

#define PST_OFFSET(OBJF, OFFS, INDEX)				\
  (ANOFFSET ((OBJF)->section_offsets, (INDEX))			\
   + ((((OFFS) == NULL)) ? 0 : ANOFFSET ((OFFS), (INDEX))))

#define PSYMTAB_TEXTLOW(PST) ((PST)->textlow_ + 0)
#define PSYMTAB_TEXTHIGH(PST) ((PST)->texthigh_ + 0)

/* Set the "textlow" field on the partial symbol table, and mark the
   field as valid.  */

static inline void
set_psymtab_textlow (struct partial_symtab *pst, CORE_ADDR low)
{
  pst->textlow_ = low;
  pst->textlow_valid = 1;
}

/* Set the "texthigh" field on the partial symbol table, and mark the
   field as valid.  */

static inline void
set_psymtab_texthigh (struct partial_symtab *pst, CORE_ADDR high)
{
  pst->texthigh_ = high;
  pst->texthigh_valid = 1;
}

#define SET_PSYMTAB_TEXTLOW(PST, V) set_psymtab_textlow ((PST), (V))
#define SET_PSYMTAB_TEXTHIGH(PST, V) set_psymtab_texthigh ((PST), (V))

extern void sort_pst_symbols (struct objfile *, struct partial_symtab *);

/* Add any kind of symbol to a psymbol_allocation_list.  */

extern void add_psymbol_to_list (const char *, int,
				 int, domain_enum,
				 enum address_class,
				 struct psymbol_allocation_list *,
				 long, CORE_ADDR,
				 enum language, struct objfile *);

extern void init_psymbol_list (struct objfile *, int);

extern struct partial_symtab *start_psymtab_common (struct objfile *,
						    const char *, CORE_ADDR,
						    struct partial_symbol **,
						    struct partial_symbol **);

extern struct partial_symtab *allocate_psymtab (const char *,
						struct objfile *)
  ATTRIBUTE_NONNULL (1);

extern void discard_psymtab (struct objfile *, struct partial_symtab *);

extern struct cleanup *make_cleanup_discard_psymtabs (struct objfile *);

/* Traverse all psymtabs in one objfile.  */

#define	ALL_OBJFILE_PSYMTABS(objfile, p) \
    for ((p) = (objfile) -> psymtabs; (p) != NULL; (p) = (p) -> next)

#endif /* PSYMPRIV_H */
