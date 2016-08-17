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


##                            Extensions                            ##

This module additionally contains some useful table-related functions
that are not part of the original `table` standard library.

*   `table.replace(t1 [, i [, j]], t2 [, m [, n]])`

    Replaces the elements with indices `i, ..., j` in `t1` with the
    elements `t2[m], ..., t2[n]`. If the two ranges have a different
    number of indices, the following elements in `t1` are shifted
    to make room or fill the gap, and `t1.n` is updated accordingly.
    The default value for `i` is `t1.n+1` (which results in appending
    to `t1`), `j` defaults to `t1.n`. Default values for `m` and `n`
    are `1` and `t2.n`, respectively.

*   `table.zip(f, t1, ...)`

    Iterates over all indices from `1` to `n` (where `n` is the
    smallest integer value of the `.n` fields of all argument tables)
    and calls the function `f` with `(t1[i], t2[i], etc.)` as
    arguments. All return values of these function calls are appended
    to a result array that is returned at the end. If `f` is `nil`,
    `table.pack` is used as a default which results in the usual
    behavior of the `zip` function known from other programming
    languages:

    ```lua
    table.zip( nil, { 1, 2, 3, n=3 }, { "a", "b", "c", n=3 } )
    --> { { 1, "a", n=2 }, { 2, "b", n=2 }, { 3, "c", n=2 }, n=3 }
    ```

    However, with a suitable choice of `f` you can for instance
    interleave values

    ```lua
    local function id( ... ) return ... end
    table.zip( id, { 1, 2, 3, n=3 }, { "a", "b", "c", n=3 } )
    --> { 1, "a", 2, "b", 3, "c", n=6 }
    ```

    or emulate the well-known `map` or `filter` functions:

    ```lua
    local function plus3( a ) return a + 3 end
    table.zip( plus3, { 1, 2, 3, n=3 } )
    --> { 4, 5, 6, n=3 }
    local function not_nil( a )
      if a ~= nil then return a end
    end
    table.zip( not_nil, { 1, nil, 2, 3, nil, 4, n=6 } )
    --> { 1, 2, 3, 4, n=4 }
    ```

*   `table.npairs(t [, i])` (or `npairs(t [, i])`)

    Returns an iterator tuple that, when used in a generic `for`-loop,
    iterates over the pairs `(1, t[1])`, `(2, t[2])`, ..., until the
    index reaches `t.n`. You may specify a starting index different
    from `1`. For convenience and consistency the `npairs` function is
    also available in the globals table.

*   `table.reverse(t [, i [, j]])`

    Reverses the elements between indices `i` and `j` in table `t`
    inplace. `i` and `j` default to `1` and `t.n`, respectively.

*   `table.rotate(t, m [, i [, j]])`

    Shifts the elements between indices `i` and `j` in table `t` `m`
    positions to the right. Elements that are shifted beyond `j` are
    reinserted at `i`. `m` may be negative to rotate in the other
    direction. Default values for `i` and `j` are `1` and `t.n`,
    respectively.

*   `table.shuffle(t [, i [, j]])`

    Randomly reorders the elements between indices `i` and `j` in
    table `t`.


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

