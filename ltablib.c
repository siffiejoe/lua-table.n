/*
** $Id: ltablib.c,v 1.93 2016/02/25 19:41:54 roberto Exp $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#define ltablib_c
#define LUA_LIB

#include "lprefix.h"


#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

/* compatibility for older Lua versions */
#include "compat-5.3/c-api/compat-5.3.h"
#ifndef LUA_MAXINTEGER
/* conservative estimate: */
#define LUA_MAXINTEGER INT_MAX
#endif


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#define TAB_R	1			/* read */
#define TAB_W	2			/* write */
#define TAB_L	4			/* length */
#define TAB_RW	(TAB_R | TAB_W)		/* read/write */


#define aux_getn(L,n,w)	(checktab(L, n, w), check_n(L, n))


/*
** Get the .n field and make sure that it is a valid length.
** Return -1 if not.
*/
static lua_Integer get_n (lua_State *L, int n) {
  lua_Integer len = 0;
  int valid = 0;
  lua_getfield(L, n, "n");
  len = lua_tointegerx(L, -1, &valid);
  lua_pop(L, 1);
  if (!valid || len < 0)
    len = -1;
  return len;
}


/*
** Get the .n field and make sure that it is a valid length.
** Raise an argument error otherwise.
*/
static lua_Integer check_n (lua_State *L, int n) {
  lua_Integer len = get_n(L, n);
  luaL_argcheck(L, len >= 0, n, "no valid '.n'");
  return len;
}


static int checkfield (lua_State *L, const char *key, int n) {
  lua_pushstring(L, key);
  return (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods)
*/
static void checktab (lua_State *L, int arg, int what) {
  if (lua_type(L, arg) != LUA_TTABLE) {  /* is it not a table? */
    int n = 1;  /* number of elements to pop */
    if (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) || checkfield(L, "__len", ++n))) {
      lua_pop(L, n);  /* pop metatable and tested metamethods */
    }
    else
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


#if defined(LUA_COMPAT_MAXN)
static int maxn (lua_State *L) {
  lua_Number max = 0;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
    if (lua_type(L, -1) == LUA_TNUMBER) {
      lua_Number v = lua_tonumber(L, -1);
      if (v > max) max = v;
    }
  }
  lua_pushnumber(L, max);
  return 1;
}
#endif


static int tinsert (lua_State *L) {
  lua_Integer e = aux_getn(L, 1, TAB_RW) + 1;  /* first empty element */
  lua_Integer pos;  /* where to insert new element */
  switch (lua_gettop(L)) {
    case 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      lua_pushinteger(L, e);
      lua_setfield(L, 1, "n"); /* set new length */
      break;
    }
    case 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      luaL_argcheck(L, 1 <= pos && pos <= e, 2, "position out of bounds");
      lua_pushinteger(L, e);
      lua_setfield(L, 1, "n"); /* set new length */
      for (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      break;
    }
    default: {
      return luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  return 0;
}


static int tremove (lua_State *L) {
  lua_Integer size = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, size);
  if (pos != size)  /* validate 'pos' if given */
    luaL_argcheck(L, 1 <= pos && pos <= size + 1, 1, "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  for ( ; pos < size; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* t[pos] = nil */
  if (pos > 0 && pos <= size) {
    lua_pushinteger(L, size-1);
    lua_setfield(L, 1, "n");
  }
  return 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
static int tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  int tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  if (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    lua_Integer size = get_n(L, tt);
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    if (size >= 0 && t+n-1 > size) {
      lua_pushinteger(L, t+n-1);
      lua_setfield(L, tt, "n" );
    }
    if (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      for (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    else {
      for (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  return 1;
}


static void addfield (lua_State *L, luaL_Buffer *b, lua_Integer i) {
  lua_geti(L, 1, i);
  if (!lua_isstring(L, -1))
    luaL_error(L, "invalid value (%s) at index %d in table for 'concat'",
                  luaL_typename(L, -1), i);
  luaL_addvalue(b);
}


static int tconcat (lua_State *L) {
  luaL_Buffer b;
  lua_Integer i, last;
  size_t lsep;
  const char *sep;
  checktab(L, 1, TAB_R);
  sep = luaL_optlstring(L, 2, "", &lsep);
  i = luaL_optinteger(L, 3, 1);
  last = luaL_opt(L, luaL_checkinteger, 4, check_n(L, 1));
  luaL_buffinit(L, &b);
  for (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  if (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  return 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

static int pack (lua_State *L) {
  int i;
  int n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_insert(L, 1);  /* put it at index 1 */
  for (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  return 1;  /* return table */
}


static int unpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, aux_getn(L, 1, TAB_R));
  if (i > e) return 0;  /* empty range */
  n = (lua_Unsigned)e - i;  /* number of elements minus 1 (avoid overflows) */
  if (n >= (unsigned int)INT_MAX  || !lua_checkstack(L, (int)(++n)))
    return luaL_error(L, "too many results to unpack");
  for (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  return (int)n;
}

/* }====================================================== */



/*
** {======================================================
** Quicksort
** (based on 'Algorithms in MODULA-3', Robert Sedgewick;
**  Addison-Wesley, 1993.)
** =======================================================
*/


/* type for array indices */
typedef unsigned int IdxT;


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#if !defined(l_randomizePivot)		/* { */

#include <time.h>

/* size of 'e' measured in number of 'unsigned int's */
#define sof(e)		(sizeof(e) / sizeof(unsigned int))

/*
** Use 'time' and 'clock' as sources of "randomness". Because we don't
** know the types 'clock_t' and 'time_t', we cannot cast them to
** anything without risking overflows. A safe way to use their values
** is to copy them to an array of a known type and use the array values.
*/
static unsigned int l_randomizePivot (void) {
  clock_t c = clock();
  time_t t = time(NULL);
  unsigned int buff[sof(c) + sof(t)];
  unsigned int i, rnd = 0;
  memcpy(buff, &c, sof(c) * sizeof(unsigned int));
  memcpy(buff + sof(c), &t, sof(t) * sizeof(unsigned int));
  for (i = 0; i < sof(buff); i++)
    rnd += buff[i];
  return rnd;
}

#endif					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#define RANLIMIT	100u


static void set2 (lua_State *L, IdxT i, IdxT j) {
  lua_seti(L, 1, i);
  lua_seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
static int sort_comp (lua_State *L, int a, int b) {
  if (lua_isnil(L, 2))  /* no function? */
    return (!lua_isnil(L, a)) &&
           (lua_isnil(L, b) ||
            lua_compare(L, a, b, LUA_OPLT));  /* a < b */
  else {  /* function */
    int res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_pop(L, 1);          /* pop result */
    return res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
static IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  for (;;) {
    /* next loop: repeat ++i while a[i] < P */
    while (lua_geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      if (i == up - 1)  /* a[i] < P  but a[up - 1] == P  ?? */
        luaL_error(L, "invalid order function for sorting");
      lua_pop(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P */
    /* next loop: repeat --j while P < a[j] */
    while (lua_geti(L, 1, --j), sort_comp(L, -3, -1)) {
      if (j < i)  /* j < i  but  a[j] > P ?? */
        luaL_error(L, "invalid order function for sorting");
      lua_pop(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    if (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_pop(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, up - 1, i);
      return i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
static IdxT choosePivot (IdxT lo, IdxT up, unsigned int rnd) {
  IdxT r4 = (up - lo) / 4;  /* range/4 */
  IdxT p = rnd % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  return p;
}


/*
** QuickSort algorithm (recursive function)
*/
static void auxsort (lua_State *L, IdxT lo, IdxT up,
                                   unsigned int rnd) {
  while (lo < up) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    lua_geti(L, 1, lo);
    lua_geti(L, 1, up);
    if (sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, up);  /* swap a[lo] - a[up] */
    else
      lua_pop(L, 2);  /* remove both values */
    if (up - lo == 1)  /* only 2 elements? */
      return;  /* already sorted */
    if (up - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    else  /* for larger intervals, it is worth a random pivot */
      p = choosePivot(lo, up, rnd);
    lua_geti(L, 1, p);
    lua_geti(L, 1, lo);
    if (sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    else {
      lua_pop(L, 1);  /* remove a[lo] */
      lua_geti(L, 1, up);
      if (sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, up);  /* swap a[up] - a[p] */
      else
        lua_pop(L, 2);
    }
    if (up - lo == 2)  /* only 3 elements? */
      return;  /* already sorted */
    lua_geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    lua_geti(L, 1, up - 1);  /* push a[up - 1] */
    set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    if (p - lo < up - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    else {
      auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    if ((up - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot();  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


static int sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  if (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < INT_MAX, 1, "array too big");
    if (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  return 0;
}

/* }====================================================== */



/*
** {======================================================
** Extensions to the official table library
** =======================================================
*/

static int treplace(lua_State *L) {
  lua_Integer len, tpos, start, end, start2, end2, i;
  len = aux_getn(L, 1, TAB_RW);
  if (lua_type(L, 2) == LUA_TNUMBER) {
    start = luaL_checkinteger(L, 2);
    luaL_argcheck(L, start >= 1 && start <= len+1, 2,
                  "index out of bounds");
    if (lua_type(L, 3) == LUA_TNUMBER) {
      end = luaL_checkinteger(L, 3);
      luaL_argcheck(L, end >= start-1 && end <= len, 3,
                    "invalid end index");
      tpos = 4;
    } else {
      end = start;
      if (end > len)
        end = len;
      tpos = 3;
    }
  } else {
    start = len+1;
    end = len;
    tpos = 2;
  }
  checktab(L, tpos, TAB_R);
  start2 = luaL_optinteger(L, tpos+1, 1);
  end2 = luaL_opt(L, luaL_checkinteger, tpos+2, check_n(L, tpos));
  luaL_argcheck(L, end2 >= start2-1, tpos+2, "invalid end index");
  if (end2-start2 > end-start) { /* array needs to grow */
    lua_pushinteger(L, len+end2-start2-end+start);
    lua_setfield(L, 1, "n");  /* t.n = number of elements */
  }
  if (start <= len) { /* replace values */
    lua_Integer shift = end2-start2-end+start;
    if (shift < 0) { /* shift to left */
      for (i = end+1; i <= len; ++i) {
        lua_geti(L, 1, i);
        lua_seti(L, 1, i+shift);
      }
      for (i = len; i > len+shift; --i) {
        lua_pushnil(L);
        lua_seti(L, 1, i);
      }
    } else if (shift != 0) { /* shift to right */
      for (i = len-shift+1; i <= len; ++i) {
        lua_geti(L, 1, i);
        lua_seti(L, 1, i+shift);
      }
      for (i = len-shift; i > end; --i) {
        lua_geti(L, 1, i);
        lua_seti(L, 1, i+shift);
      }
    }
  }
  /* copy from list2 to list1 */
  for (i = start2; i <= end2; ++i) {
    lua_geti(L, tpos, i);
    lua_seti(L, 1, start+i-start2);
  }
  lua_settop(L, 1);
  /* array must shrink */
  if (end2-start2 < end-start) {
    lua_pushinteger(L, len+end2-start2-end+start);
    lua_setfield(L, 1, "n");  /* t.n = number of elements */
  }
  return 1;
}


static void check_callable(lua_State *L, int idx) {
  int c = 0;
  switch (lua_type(L, idx)) {
    case LUA_TFUNCTION:
      c = 1;
      break;
    case LUA_TUSERDATA: /* fall through */
    case LUA_TTABLE:
      if (luaL_getmetafield(L, idx, "__call")) {
        lua_pop(L, 1);
        c = 1;
      }
      break;
  }
  luaL_argcheck(L, c, idx, "callable value expected");
}


static int zipper (lua_State *L) {
  int i = 1, top = lua_gettop(L);
  lua_createtable(L, top, 1);
  lua_pushinteger(L, top);
  lua_setfield(L, -2, "n");
  for (i = 1; i <= top; ++i) {
    lua_pushvalue(L, i);
    lua_rawseti(L, -2, i);
  }
  return 1;
}


LUA_KFUNCTION(tzipk) {
  lua_Integer i = 1, j = 1, len;
  int k, n, top;
  (void)status;
  switch (ctx) {
    case 0:
      if (lua_isnoneornil(L, 1)) {
        lua_pushcfunction(L, zipper);
        lua_replace(L, 1);
      } else
        check_callable(L, 1);
      n = lua_gettop(L)-1;
      len = aux_getn(L, 2, TAB_R);
      for (k = 3; k <= n+1; ++k) {
        lua_Integer l = aux_getn(L, k, TAB_R);
        if (l < len)
          len = l;
      }
      lua_pushinteger(L, n); /* store number of arrays */
      lua_insert(L, 2);
      lua_pushinteger(L, 1); /* store current iteration index */
      lua_pushvalue(L, -1); /* store current target index */
      lua_pushinteger(L, len); /* store length on stack */
      lua_createtable(L, 0, 1); /* result table */
      luaL_checkstack(L, n+2+LUA_MINSTACK, "zip");
      while (i <= len) { /* func, n, t_1, ..., t_n, i, j, len, rt */
        lua_pushvalue(L, 1);
        for (k = 3; k <= n+2; ++k) {
          lua_pushvalue(L, n+3);
          lua_gettable(L, k);
        }
        lua_callk(L, n, LUA_MULTRET, 1, tzipk);
    case 1: /* func, n, t_1, ..., t_n, i, j, len, rt, r_1, ..., r_m */
        n = lua_tointeger(L, 2);
        i = lua_tointeger(L, n+3);
        j = lua_tointeger(L, n+4);
        len = lua_tointeger(L, n+5);
        top = lua_gettop(L);
        luaL_checkstack(L, 1, "zip");
        for (k = n+7; k <= top; ++k) {
          lua_pushvalue(L, k);
          lua_rawseti(L, n+6, j++);
        }
        lua_settop(L, n+6); /* remove results r_1, ..., r_m */
        lua_pushinteger(L, ++i);
        lua_replace(L, n+3); /* update i */
        lua_pushinteger(L, j);
        lua_replace(L, n+4); /* update j */
      }
  }
  lua_pushinteger(L, j-1);
  lua_setfield(L, -2, "n");
  return 1;
}

static int tzip (lua_State *L) {
  return tzipk(L, 0, 0);
}


static int npairs_iterator (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_checkinteger(L, 2);
  if (i >= n)
    return 0;
  lua_pushinteger(L, ++i);
  lua_pushvalue(L, -1);
  lua_gettable(L, 1);
  return 2;
}


static int npairs (lua_State *L) {
  lua_Integer s;
  (void)aux_getn(L, 1, TAB_R); /* check early */
  s = luaL_optinteger(L, 2, 1);
  lua_pushcfunction(L, npairs_iterator);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, s-1);
  return 3;
}


static void reverse (lua_State *L, lua_Integer a, lua_Integer b ) {
  for( ; a < b; ++a, --b ) {
    lua_geti( L, -1, a );
    lua_geti( L, -2, b );
    lua_seti( L, -3, a );
    lua_seti( L, -2, b );
  }
}


static int treverse (lua_State *L) {
  lua_Integer begin, end;
  checktab(L, 1, TAB_RW);
  begin = luaL_optinteger(L, 2, 1);
  end = luaL_opt(L, luaL_checkinteger, 3, check_n(L, 1));
  lua_settop(L, 1);
  reverse(L, begin, end);
  return 0;
}


static int trotate (lua_State *L) {
  lua_Integer n, begin, end;
  checktab(L, 1, TAB_RW);
  n = -luaL_checkinteger(L, 2);
  begin = luaL_optinteger(L, 3, 1);
  end = luaL_opt(L, luaL_checkinteger, 4, check_n(L, 1));
  if (end > begin) {
    n %= end - begin + 1;
    if (n < 0)
      n += end - begin + 1;
    if (n != 0) {
      lua_settop(L, 1);
      reverse(L, begin, begin+n-1);
      reverse(L, begin+n, end);
      reverse(L, begin, end);
    }
  }
  return 0;
}


/* try to guess which PRNG Lua uses */
#if !defined(l_rand)
#if defined(unix) || defined(__unix) || defined(__unix__) || \
    (defined(__APPLE__) && defined(__MACH__)) || \
    HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(_POSIX_VERSION)
#define l_rand()   random()
#define L_RANDMAX  2147483647
#else
#define l_rand()   rand()
#define L_RANDMAX  RAND_MAX
#endif
#endif

static int tshuffle (lua_State *L) {
  lua_Integer begin, end;
  checktab(L, 1, TAB_RW);
  begin = luaL_optinteger(L, 2, 1);
  end = luaL_opt(L, luaL_checkinteger, 3, check_n(L, 1));
  while (end >= begin) {
    double f = l_rand() * (1.0/(L_RANDMAX+1.0));
    lua_Integer j = begin + (lua_Integer)(f * (end-begin+1));
    lua_geti(L, 1, end);
    lua_geti(L, 1, j);
    lua_seti(L, 1, end);
    lua_seti(L, 1, j);
    --end;
  }
  return 0;
}

/* }====================================================== */



static const luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
#if defined(LUA_COMPAT_MAXN)
  {"maxn", maxn},
#endif
  {"insert", tinsert},
  {"pack", pack},
  {"unpack", unpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {"replace", treplace},
  {"zip", tzip},
  {"npairs", npairs},
  {"reverse", treverse},
  {"rotate", trotate},
  {"shuffle", tshuffle},
  {NULL, NULL}
};


#ifndef TABLE_N_API
#ifdef _WIN32
#define TABLE_N_API __declspec(dllexport)
#else
#define TABLE_N_API
#endif
#endif

TABLE_N_API int luaopen_table_n (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  /* _G.npairs = table.npairs */
  lua_getfield(L, -1, "npairs");
  lua_setglobal(L, "npairs");
  return 1;
}

