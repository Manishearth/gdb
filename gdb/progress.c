/* Progress reporting.

   Copyright (C) 2012 Free Software Foundation, Inc.

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

#include "defs.h"
#include "progress.h"
#include <string.h>

struct progress_info
{
  int printing;
  char *name;
  double last_value;
  struct progress_info *prev;
};

static struct progress_info *progress_stack;

static int progress_printing_count;

static void
maybe_erase_line (void)
{
  int i;

  if (progress_printing_count == 0)
    return;

  printf ("\r");
  for (i = 0; i < 79; ++i)
    printf (" ");
  printf ("\r");
}

void
progress_initialize (const char *name, int printing)
{
  struct progress_info *result = XNEW (struct progress_info);
  int len = strlen (name);

  result->printing = printing;
  result->last_value = 0;

  /* FIXME chars_per_line, isatty, etc */
  if (len > 27)
    {
      name += len - 27;
      result->name = xstrprintf ("...%s", name);
    }
  else
    result->name = xstrdup (name);

  if (printing)
    {
      maybe_erase_line ();
      ++progress_printing_count;
    }

  result->prev = progress_stack;
  progress_stack = result;

  progress_notify (0);
}

void
progress_notify (double howmuch)
{
  int i, max;

  progress_stack->last_value = howmuch;
  if (!progress_stack->printing)
    return;

  printf ("\r%30s ", progress_stack->name);
  max = (int) ((80 - 31) * howmuch);

  for (i = 0; i < max; ++i)
    printf ("#");
  fflush (stdout);
}

void
progress_done (void)
{
  struct progress_info *info = progress_stack;

  /* Dunno.  */
  progress_notify (1.0);

  progress_stack = info->prev;

  if (info->printing)
    {
      --progress_printing_count;
      printf ("\n");
    }
  xfree (info->name);
  xfree (info);

  if (progress_stack != NULL)
    progress_notify (progress_stack->last_value);
}
