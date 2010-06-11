/* Public partial symbol table definitions.

   Copyright (C) 2009, 2010 Free Software Foundation, Inc.

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

#ifndef PSYMTAB_H
#define PSYMTAB_H

void map_partial_symbol_names (void (*) (const char *, void *), void *);

void map_partial_symbol_filenames (void (*) (const char *, const char *,
					     void *),
				   void *);

struct psymtab_state;

/* Allocate a new psymtab_state on OBSTACK.  The new psymtab_state is
   initialized.  */
struct psymtab_state *allocate_psymtab_state (struct obstack *obstack);

/* Free any memory allocated by the psymtab_state object.  After this
   call, the contents of the object are invalid.  The object itself is
   not deallocated, however.  */
void destroy_psymtab_state (struct psymtab_state *state);

extern const struct quick_symbol_functions psym_functions;

extern struct quick_symbol_functions dwarf2_background_functions;

#endif /* PSYMTAB_H */
