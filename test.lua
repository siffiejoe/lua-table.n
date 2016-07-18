#!/usr/bin/env lua

local table = require( "table.n" )

-- sample tables
local t1, t2, t3, t4, t5
local function reset()
  t1 = { 1 }
  t2 = table.pack()
  t3 = table.pack( nil, nil, nil )
  t4 = table.pack( 1 )
  t5 = table.pack( nil, 3, nil, 1, nil, 2, nil )
end
reset()

local function p( t, ... )
  print( table.unpack( t, ... ) )
end


print( "those should all fail:" )
print( pcall( table.concat, t1 ) )
print( pcall( table.insert, t1, 1 ) )
print( pcall( table.remove, t1 ) )
print( pcall( table.sort, t1 ) )
print( pcall( table.unpack, t1 ) )
print( pcall( table.replace, t1, {} ) )
print( pcall( table.replace, t2, {} ) )
print( pcall( table.replace, {1,2,3,n=3}, 5, {"a","b","c"} ) )
print( pcall( table.replace, true ) )
print( pcall( table.replace, {1,2,3,n=3}, true, {"a","b",n=2} ) )
print( pcall( table.replace, {1,2,3,n=3}, {"a","b",n=2}, true ) )
print( pcall( table.zip, {}, {} ) )
print( pcall( table.zip, type, {} ) )
print( pcall( table.zip, type, {n=0}, {} ) )
print( pcall( table.npairs, {} ) )
print( pcall( table.reverse, {} ) )
print( pcall( table.rotate, {}, 1 ) )
print( pcall( table.shuffle, {} ) )


print( "table.unpack() ..." )
p( t1, 1, 1 )
p( t2 )
p( t3 )
p( t4 )
p( t5 )


print( "table.concat() ..." )
print( table.concat( t1, ", ", 1, 1 ) )
print( table.concat( t4, ", " ) )


print( "table.insert() ..." )
table.insert( t2, 1 )
table.insert( t2, 1, 2 )
p( t2 )
table.insert( t3, 1 )
table.insert( t3, 2, 2 )
p( t3 )
reset()


print( "table.move() ..." )
table.move( t1, 1, 1, 2 )
p( t1, 1, 2 )
table.move( t2, 1, 3, 4 )
p( t2 )
table.move( t3, 1, 1, 4 )
p( t3 )
table.move( t5, 2, 4, 3 )
p( t5 )
reset()


print( "table.remove() ..." )
table.remove( t2 )
p( t2 )
table.remove( t2, 1 )
p( t2 )
table.remove( t3 )
table.remove( t3, 1 )
p( t3 )
table.remove( t5 )
table.remove( t5, 2 )
table.remove( t5, 1 )
p( t5 )
reset()


print( "table.sort() ..." )
table.sort( t2 )
p( t2 )
table.sort( t3 )
p( t3 )
table.sort( t5 )
p( t5 )
reset()


print( "table.replace() ..." )
local t = {1,2,3,4,n=4}
table.replace( t, {"a","b","c",n=3} )
p( t )
t = {1,2,3,4,n=4}
table.replace( t, 2, 3, {"a","b","c","d"}, 2, 4 )
p( t )
t = {1,2,3,4,n=4}
table.replace( t, 2, 3, {"a","b","c"}, 3, 3 )
p( t )
t = {1,2,3,4,5,n=5}
table.replace( t, 2, 4, {"a","b"}, 1, 1 )
p( t )
t = {1,2,3,4,n=4}
table.replace( t, 2, 1, {"a","b","c"}, 3, 3 )
p( t )
t = {1,2,3,4,n=4}
table.replace( t, 1, 0, {"a","b","c",n=3} )
p( t )
t = {1,2,3,4,n=4}
table.replace( t, 1, 0, {"a","b",n=2} )
p( t )
table.replace( t3, 2, 1, t1, 1, 1 )
p( t3 )
table.replace( t4, 1, 1, t1, 1, 0 )
p( t4 )
table.replace( t5, 3, {"a","b",n=2} )
p( t5 )
reset()


print( "table.zip() ..." )
local function is_nil( v )
  if v == nil then return nil end
end
local function is_not_nil( v )
  if v ~= nil then return v end
end
p( table.zip( is_nil, t3 ) )
p( table.zip( is_not_nil, t3 ) )
p( table.zip( is_not_nil, t5 ) )
p( table.zip( type, t3 ) )
p( table.zip( type, t4 ) )
p( table.zip( type, t5 ) )
local function add( a ) return a+3 end
p( table.zip( add, {1,2,3,4,n=3} ) )
local function id( ... ) return ...  end
p( table.zip( id, t3, t5 ) )
local x = table.zip( nil, t5, t3 )
for i = 1, x.n do
  io.write( i, "\t" )
  p( x[ i ] )
end


print( "table.npairs() ..." )
for i,v in table.npairs( t3 ) do
  print( i, v )
end
for i,v in table.npairs( t5, 2 ) do
  print( i, v )
end


print( "table.reverse() ..." )
t1[ 2 ] = 2
table.reverse( t1, 1, 2 )
p( t1, 1, 2 )
table.reverse( t5, 2, 7 )
p( t5 )
reset()
table.reverse( t5 )
p( t5 )
reset()


print( "table.rotate() ..." )
t1[ 2 ] = 2
table.rotate( t1, 3, 1, 2 )
p( t1, 1, 2 )
table.rotate( t5, 3, 2, 7 )
p( t5 )
reset()
table.rotate( t5, -1 )
p( t5 )
reset()


print( "table.shuffle() ..." )
t1[ 2 ] = 2
t1[ 3 ] = 3
table.shuffle( t1, 1, 3 )
p( t1, 1, 3 )
table.shuffle( t1, 1, 3 )
p( t1, 1, 3 )
table.shuffle( t1, 1, 3 )
p( t1, 1, 3 )
table.shuffle( t1, 1, 3 )
p( t1, 1, 3 )
table.shuffle( t1, 1, 3 )
p( t1, 1, 3 )
table.shuffle( t5 )
p( t5 )
table.shuffle( t5 )
p( t5 )
table.shuffle( t5 )
p( t5 )
table.shuffle( t5, 3, 6 )
p( t5 )
table.shuffle( t5, 3, 6 )
p( t5 )
table.shuffle( t5, 3, 6 )
p( t5 )
table.shuffle( t5, 3, 6 )
p( t5 )

