package="table.n"
version="scm-0"
source = {
  url = "git+https://github.com/siffiejoe/lua-table.n.git",
}
description = {
  summary = "Clone of Lua's table library that uses '.n'",
  detailed = [[
    Lua uses "sequences", i.e. tables with consecutive integer
    keys and non-nil values, as arrays. But since one cannot
    efficiently check whether a table is a sequence or not (and
    in fact the default table library doesn't try), this leads
    to some surprising bugs. Using an explicit `n` field to
    denote table lengths is an alternative approach that's
    (hopefully) easier to grasp and less error prone.
  ]],
  homepage = "https://github.com/siffiejoe/lua-table.n/",
  license = "MIT"
}
dependencies = {
  "lua >= 5.3, < 5.4"
}
build = {
  type = "builtin",
  modules = {
    ["table.n"]                   = "ltablib.c",
  }
}

