# Copyright (C) 2016 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gdb

UNWRAP_MAP = {
  'core::cell::Cell<' : 'value',
  'core::cell::UnsafeCell<' : 'value',
  'core::cell::RefCell<' : 'value',
  'alloc::rc::Rc<' : 'ptr',
  'alloc::rc::RcBox<' : 'value',
  'core::ptr::Shared<' : 'pointer',
  'alloc::arc::Arc<' : 'ptr',
  'alloc::arc::ArcInner<' : 'data',
  'std::sync::mutex::Mutex<' : 'data',
  'core::nonzero::NonZero<' : '__0',
}

class Unwrap(gdb.Function):
    """Return the string representation of a value.

Usage:
  $_as_string(value)

Arguments:

  value: A gdb.Value.

Returns:
  The string representation of the value.
"""

    def __init__(self):
        super(Unwrap, self).__init__("unwrap")

    def invoke(self, val):
        import code
        # code.interact(local=locals())
        if val.type.code is gdb.TYPE_CODE_PTR:
            return self.invoke(val.referenced_value())

        for k,v in UNWRAP_MAP.iteritems():
            if val.type.name.startswith(k):
                try:
                    inner = val[v]
                    return self.invoke(inner)
                except gdb.error:
                    return val
        return val

Unwrap()
