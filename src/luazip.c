/*
 LuaZip - Reading files inside zip files.
 https://github.com/mpeterv/luazip

 Author: Danilo Tuler
 Maintainer: Peter Melnichenko
 Copyright (c) 2003-2007 Kepler Project
 Copyright (c) 2016-2017 Peter Melnichenko
*/

#include <string.h>
#include <stdlib.h>
#include "zzip/zzip.h"
#include "lualib.h"
#include "lauxlib.h"

#define ZIPFILEHANDLE    "lzipFile"
#define ZIPINTERNALFILEHANDLE  "lzipInternalFile"
#define LUAZIP_MAX_EXTENSIONS 32

#ifndef luaL_reg
#define luaL_reg luaL_Reg
#endif
#ifndef luaL_optlong
#define luaL_optlong luaL_optinteger
#endif

#if LUA_VERSION_NUM == 501
#define get_length lua_objlen
#else
#define get_length lua_rawlen
#endif

static void set_funcs (lua_State *L, const luaL_Reg *lib) {
  for (; lib->name != NULL; lib++) {
    lua_pushstring(L, lib->name);
    lua_pushcfunction(L, lib->func);
    lua_settable(L, -3);
  }
}

static int pushresult (lua_State *L, int i, const char *filename) {
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, zzip_strerror(zzip_errno(errno)));
    else
      lua_pushfstring(L, "%s", zzip_strerror(zzip_errno(errno)));
    lua_pushnumber(L, zzip_errno(errno));
    return 3;
  }
}

static ZZIP_DIR** topfile (lua_State *L, int findex) {
  ZZIP_DIR** f = (ZZIP_DIR**)luaL_checkudata(L, findex, ZIPFILEHANDLE);
  if (f == NULL) luaL_argerror(L, findex, "bad zip file");
  return f;
}

static ZZIP_DIR* tofile (lua_State *L, int findex) {
  ZZIP_DIR** f = topfile(L, findex);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed zip file");
  return *f;
}

static ZZIP_FILE** topinternalfile (lua_State *L, int findex) {
  ZZIP_FILE** f = (ZZIP_FILE**)luaL_checkudata(L, findex, ZIPINTERNALFILEHANDLE);
  if (f == NULL) luaL_argerror(L, findex, "bad zip file");
  return f;
}

static ZZIP_FILE* tointernalfile (lua_State *L, int findex) {
  ZZIP_FILE** f = topinternalfile(L, findex);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed zip file");
  return *f;
}

/*
** When creating file handles, always creates a `closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
static ZZIP_DIR** newfile (lua_State *L) {
  ZZIP_DIR** pf = (ZZIP_DIR**)lua_newuserdata(L, sizeof(ZZIP_DIR*));
  *pf = NULL;  /* file handle is currently `closed' */
  luaL_getmetatable(L, ZIPFILEHANDLE);
  lua_setmetatable(L, -2);
  return pf;
}

static ZZIP_FILE** newinternalfile (lua_State *L) {
  ZZIP_FILE** pf = (ZZIP_FILE**)lua_newuserdata(L, sizeof(ZZIP_FILE*));
  *pf = NULL;  /* file handle is currently `closed' */
  luaL_getmetatable(L, ZIPINTERNALFILEHANDLE);
  lua_setmetatable(L, -2);
  return pf;
}


static int zip_open (lua_State *L) {
  const char *zipfilename = luaL_checkstring(L, 1);
  /*const char *mode = luaL_optstring(L, 2, "r");*/

  ZZIP_DIR** pf = newfile(L);
  *pf = zzip_dir_open(zipfilename, 0);
  if (*pf == NULL)
  {
    lua_pushnil(L);
    lua_pushfstring(L, "could not open file `%s'", zipfilename);
    return 2;
  }
  return 1;
}

static int zip_close (lua_State *L) {
  ZZIP_DIR* f = tofile(L, 1);
  if (zzip_dir_close(f) == 0)
  {
    *(ZZIP_DIR**)lua_touserdata(L, 1) = NULL; /* mark file as close */
    lua_pushboolean(L, 1);
  }
  else
    lua_pushboolean(L, 0);
  return 1;
}

static int f_open (lua_State *L) {
  ZZIP_DIR* uf = tofile(L, 1);
  const char *filename = luaL_checkstring(L, 2);
  /*const char *mode = luaL_optstring(L, 3, "r");*/
  ZZIP_FILE** inf = newinternalfile(L);

  *inf = zzip_file_open(uf, filename, 0);
  if (*inf)
    return 1;

  lua_pushnil(L);
  lua_pushfstring(L, "could not open file `%s'", filename);
  return 2;
}

/*

*/
static int zip_openfile (lua_State *L) {
  ZZIP_FILE** inf;

  const char * ext2[LUAZIP_MAX_EXTENSIONS+1];
  zzip_strings_t *ext = ext2;

  const char *filename = luaL_checkstring(L, 1);
  /*const char *mode = luaL_optstring(L, 2, "r");*/

  inf = newinternalfile(L);

  if (lua_isstring(L, 2))
  {
    /* creates a table with the string as the first and only (numerical) element */
    lua_newtable(L);
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, 1);

    /* replaces the string by the table with the string inside */
    lua_replace(L, 2);
  }
  
  if (lua_istable(L, 2))
  {
    int i, m, n;

    /* how many extension were specified? */
    n = get_length(L, 2);

    if (n > LUAZIP_MAX_EXTENSIONS)
    {
      luaL_error(L, "too many extensions specified");
    }

    for (i = 0, m = 0; i < n; i++)
    {
      lua_rawgeti(L, 2, i+1);
      if (lua_isstring(L, -1))
      {
        /* luazip specifies "zip" as the extension, but zziplib expects ".zip" */
        lua_pushstring(L, ".");
        lua_insert(L, -2);
        lua_concat(L, 2);

        ext2[m] = lua_tostring(L, -1);
        m++;
      }
      lua_pop(L, 1);
    }
    ext2[m] = 0;

    *inf = zzip_open_ext_io(filename, 0, 0664, ext, 0);
  }
  else
  {
    *inf = zzip_open(filename, 0);
  }

  if (*inf)
    return 1;

  lua_pushnil(L);
  lua_pushfstring(L, "could not open file `%s'", filename);
  return 2;
}

static int zip_type (lua_State *L) {
  ZZIP_DIR** f = (ZZIP_DIR**)luaL_checkudata(L, 1, ZIPFILEHANDLE);
  if (f == NULL) lua_pushnil(L);
  else if (*f == NULL)
    lua_pushliteral(L, "closed zip file");
  else
    lua_pushliteral(L, "zip file");
  return 1;
}

static int zip_tostring (lua_State *L) {
  char buff[32];
  ZZIP_DIR** f = topfile(L, 1);
  if (*f == NULL)
    strcpy(buff, "closed");
  else
    sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "zip file (%s)", buff);
  return 1;
}

static int ff_tostring (lua_State *L) {
  char buff[32];
  ZZIP_FILE** f = topinternalfile(L, 1);
  if (*f == NULL)
    strcpy(buff, "closed");
  else
    sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "file in zip file (%s)", buff);
  return 1;
}

static int zip_gc (lua_State *L) {
  ZZIP_DIR**f = topfile(L, 1);
  if (*f != NULL)  /* ignore closed files */
    zip_close(L);
  return 0;
}

static int zip_readfile (lua_State *L) {
  ZZIP_DIRENT* ent = NULL;
  ZZIP_DIR* uf = NULL;

  uf = *(ZZIP_DIR**)lua_touserdata(L, lua_upvalueindex(1));
  if (uf == NULL)  /* file is already closed? */
    luaL_error(L, "file is already closed");

  ent = zzip_readdir(uf);

  if (ent == NULL)
    return 0;

  lua_newtable(L);
  lua_pushstring(L, "compressed_size"); lua_pushnumber(L, ent->d_csize); lua_settable(L, -3);
  lua_pushstring(L, "compression_method"); lua_pushnumber(L, ent->d_compr); lua_settable(L, -3);
  lua_pushstring(L, "uncompressed_size"); lua_pushnumber(L, ent->st_size); lua_settable(L, -3);
  lua_pushstring(L, "filename"); lua_pushstring(L, ent->d_name); lua_settable(L, -3);

  return 1;
}

static int f_files (lua_State *L) {
  ZZIP_DIR *f = tofile(L, 1);
  zzip_rewinddir(f);
  lua_pushliteral(L, ZIPFILEHANDLE);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_pushcclosure(L, zip_readfile, 2);
  return 1;
}

static int aux_close (lua_State *L) {
  ZZIP_FILE *f = tointernalfile(L, 1);
  int ok = (zzip_fclose(f) == 0);
  if (ok)
    *(ZZIP_FILE **)lua_touserdata(L, 1) = NULL;  /* mark file as closed */
  return ok;
}

static int ff_close (lua_State *L) {
  return pushresult(L, aux_close(L), NULL);
}

static int ff_gc (lua_State *L) {
  ZZIP_FILE**f = topinternalfile(L, 1);
  if (*f != NULL)  /* ignore closed files */
    aux_close(L);
  return 0;
}

static int test_eof (lua_State *L, ZZIP_FILE *f) {
  int no_eof;

  if (zzip_file_real(f)) {
    char c;
    no_eof = zzip_read(f, &c, 1) != 0;

    if (no_eof) {
      zzip_seek(f, -1, SEEK_CUR);
    }
  } else {
    ZZIP_STAT z_stat;
    zzip_file_stat(f, &z_stat);
    no_eof = zzip_tell(f) != z_stat.st_size;
  }

  lua_pushliteral(L, "");
  return no_eof;
}

static int read_line (lua_State *L, ZZIP_FILE *f) {
  luaL_Buffer b;
  int read_something = 0;
  luaL_buffinit(L, &b);

  for (;;) {
    int i;
    char *buff = luaL_prepbuffer(&b);

    for (i = 0; i < LUAL_BUFFERSIZE; i++) {
      int c = '\0';

      if ((zzip_read(f, &c, 1) != 1) || (c == '\r') || (c == '\n')) {
        luaL_addsize(&b, i);
        luaL_pushresult(&b);
		if (c == '\r') zzip_read(f, &c, 1);	// skip next char \n (in windows)
        return read_something || (c == '\r') || (c == '\n');
      }

      buff[i] = c;
      read_something = 1;
    }
  }
}

static int read_chars (lua_State *L, ZZIP_FILE *f, size_t n) {
  size_t rlen;  /* how much to read */
  size_t nr;  /* number of chars actually read */
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  rlen = LUAL_BUFFERSIZE;  /* try to read that much each time */
  do {
    char *p = luaL_prepbuffer(&b);
    if (rlen > n) rlen = n;  /* cannot read more than asked */
    nr = zzip_fread(p, sizeof(char), rlen, f);
    luaL_addsize(&b, nr);
    n -= nr;  /* still have to read `n' chars */
  } while (n > 0 && nr == rlen);  /* until end of count or eof */
  luaL_pushresult(&b);  /* close buffer */
  return (n == 0 || get_length(L, -1) > 0);
}

static int g_read (lua_State *L, ZZIP_FILE *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)lua_tonumber(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = lua_tostring(L, n);
        luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
        switch (p[1]) {
          case 'l':  /* line */
            success = read_line(L, f);
            break;
          case 'a':  /* file */
            read_chars(L, f, ~((size_t)0));  /* read MAX_SIZE_T chars */
            success = 1; /* always success */
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}

static int ff_read (lua_State *L) {
  return g_read(L, tointernalfile(L, 1), 2);
}

static int zip_readline (lua_State *L) {
  ZZIP_FILE *f = *(ZZIP_FILE **)lua_touserdata(L, lua_upvalueindex(1));
  if (f == NULL)  /* file is already closed? */
    luaL_error(L, "file is already closed");
  return read_line(L, f);
}

static void aux_lines (lua_State *L, int idx) {
  lua_pushvalue(L, idx);
  lua_pushcclosure(L, zip_readline, 1);
}

static int ff_lines (lua_State *L) {
  tointernalfile(L, 1);  /* check that it's a valid file handle */
  aux_lines(L, 1);
  return 1;
}

static int ff_seek (lua_State *L) {
  static const int mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  ZZIP_FILE *f = tointernalfile(L, 1);
  long offset = luaL_optlong(L, 3, 0);
#if ! defined (LUA_VERSION_NUM) || LUA_VERSION_NUM < 501
  int op = luaL_findstring(luaL_optstring(L, 2, "cur"), modenames);
  luaL_argcheck(L, op != -1, 2, "invalid mode");
#else
  int op = luaL_checkoption(L, 2, "cur", modenames);
#endif
  op = zzip_seek(f, offset, mode[op]);
  if (op < 0)
    return pushresult(L, 0, NULL);  /* error */
  else {
    lua_pushnumber(L, zzip_tell(f));
    return 1;
  }
}

static const luaL_reg ziplib[] = {
  {"open", zip_open},
  {"close", zip_close},
  {"type", zip_type},
  {"openfile", zip_openfile},
  {NULL, NULL}
};

static const luaL_reg flib[] = {
  {"open", f_open},
  {"close", zip_close},
  {"files", f_files},
  {"__gc", zip_gc},
  {"__tostring", zip_tostring},
  {NULL, NULL}
};

static const luaL_reg fflib[] = {
  {"read", ff_read},
  {"close", ff_close},
  {"seek", ff_seek},
  {"lines", ff_lines},
  {"__gc", ff_gc},
  {"__tostring", ff_tostring},
  {NULL, NULL}
};


/*
** Assumes the table is on top of the stack.
*/
static void set_info (lua_State *L) {
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2003-2007 Kepler Project");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "Reading files inside zip files");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "LuaZip 1.2.7");
	lua_settable (L, -3);
}

static void createmeta (lua_State *L) {
  luaL_newmetatable(L, ZIPFILEHANDLE);  /* create new metatable for file handles */
  /* file methods */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  set_funcs(L, flib);

  luaL_newmetatable(L, ZIPINTERNALFILEHANDLE); /* create new metatable for internal file handles */
  /* internal file methods */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  set_funcs(L, fflib);
}

LUALIB_API int luaopen_zip (lua_State *L) {
  createmeta(L);
  lua_newtable(L);
  set_funcs(L, ziplib);
  set_info(L);
  lua_pushvalue(L, -1);
  lua_setglobal(L, LUA_ZIPLIBNAME);
  return 1;
}
