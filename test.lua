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


print( "those should all fail:" )
print( pcall( table.concat, t1 ) )
print( pcall( table.insert, t1, 1 ) )
print( pcall( table.remove, t1 ) )
print( pcall( table.sort, t1 ) )
print( pcall( table.unpack, t1 ) )


print( "table.concat() ..." )
print( table.concat( t1, ", ", 1, 1 ) )
print( table.concat( t4, ", " ) )


print( "table.insert() ..." )
table.insert( t2, 1 )
table.insert( t2, 1, 2 )
for i = 1, t2.n do
  print( i, t2[ i ] )
end
table.insert( t3, 1 )
table.insert( t3, 2, 2 )
for i = 1, t3.n do
  print( i, t3[ i ] )
end
reset()


print( "table.move() ..." )
table.move( t1, 1, 1, 2 )
for i = 1, 2 do
  print( i, t1[ i ] )
end
table.move( t2, 1, 3, 4 )
for i = 1, t2.n do
  print( i, t2[ i ] )
end
table.move( t3, 1, 1, 4 )
for i = 1, t3.n do
  print( i, t3[ i ] )
end
table.move( t5, 2, 4, 3 )
for i = 1, t5.n do
  print( i, t5[ i ] )
end
reset()


print( "table.remove() ..." )
table.remove( t2 )
for i = 1, t2.n do
  print( i, t2[ i ] )
end
table.remove( t2, 1 )
for i = 1, t2.n do
  print( i, t2[ i ] )
end
table.remove( t3 )
table.remove( t3, 1 )
for i = 1, t3.n do
  print( i, t3[ i ] )
end
table.remove( t5 )
table.remove( t5, 2 )
table.remove( t5, 1 )
for i = 1, t5.n do
  print( i, t5[ i ] )
end
reset()


print( "table.sort() ..." )
table.sort( t2 )
for i = 1, t2.n do
  print( i, t2[ i ] )
end
table.sort( t3 )
for i = 1, t3.n do
  print( i, t3[ i ] )
end
table.sort( t5 )
for i = 1, t5.n do
  print( i, t5[ i ] )
end
reset()


print( "table.unpack() ..." )
print( table.unpack( t1, 1, 1 ) )
print( table.unpack( t2 ) )
print( table.unpack( t3 ) )
print( table.unpack( t4 ) )
print( table.unpack( t5 ) )

