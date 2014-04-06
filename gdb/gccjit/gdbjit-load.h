/* Header file to load module for 'expression' command.
   Copyright (C) 2014 Free Software Foundation, Inc.

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

#ifndef GDB_GCCJIT_GDBJIT_LOAD_H
#define GDB_GCCJIT_GDBJIT_LOAD_H

struct gdbjit_module
{
  /* objfile for the compiled JIT module.  */
  struct objfile *objfile;

  /* Inferior function address.  */
  CORE_ADDR func_addr;
  
  /* Inferior registers address or NULL if the inferior function does not
     require any.  */
  CORE_ADDR regs_addr;

  /* This variable is NULL-initialized by gdbjit_load.
     It is required by gdbjit_run.  */
  struct gdb_gcc_instance *compiler;
};

extern struct gdbjit_module gdbjit_load (const char *object_file);

#endif /* GDB_GCCJIT_GDBJIT_LOAD_H */
