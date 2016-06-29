#            table.n -- Alternative Table Library for Lua            #

Lua doesn't have a native array type. Instead it uses the concept of
"sequences" (tables containing small consecutive integer keys with
non-`nil` values) to represent arrays. One drawback is that it is
difficult to check whether a table is a valid sequence (and neither
the length operator `#` nor the default `table` library do check), and
this may lead to surprising bugs. This module is a drop-in replacement
of the default table library that doesn't use sequences or the `#`
operator, but instead requires you to explicitly set and maintain a
length field `n` in your arrays (the functions in this module will
update `n` for you, though).


##                           Installation                           ##

    luarocks install --server=http://luarocks.org/dev table.n

If you want to install manually: Compile the file `ltablib.c` into a
shared library `table\n.dll` or `table/n.so` and put it somewhere into
your `package.cpath`.


##                             Contact                              ##

Philipp Janda, siffiejoe(a)gmx.net

Comments and feedback are always welcome.


##                             License                              ##

**table.n** is a slightly modified copy of the table library in Lua
5.3 and as such is distributed under the same license (MIT). The full
license text follows:

    Copyright (C) 1994-2016 Lua.org, PUC-Rio.

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


This module also uses code from the [Compat-5.3][1] project, which is
available under the same license (MIT).

  [1]: https://github.com/keplerproject/lua-compat-5.3

