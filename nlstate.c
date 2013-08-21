/*
 * Numlua state
 * Author: Luis Carvalho <lexcarvalho at gmail.com>
 * See copyright in nlstate.h
*/

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>

#if LUA_VERSION_NUM <= 501
#define luaL_len luaL_getn
#endif

static int state_print (lua_State *L) {
  int i, n = lua_gettop(L); /* nargs */
  const char *s;
  luaL_Buffer b;
  /* process input */
  luaL_buffinit(L, &b);
  lua_getglobal(L, "tostring");
  for (i = 1; i <= n; i++) {
    lua_pushvalue(L, -1); /* tostring */
    lua_pushvalue(L, i); /* arg */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);
    if (s == NULL)
      return luaL_error(L, "cannot convert to string");
    if (i > 1) luaL_addchar(&b, '\t');
    luaL_addlstring(&b, s, strlen(s));
    lua_pop(L, 1);
  }
  luaL_pushresult(&b);
  s = lua_tostring(L, -1);
  /* update msg buffer */
  lua_pushvalue(L, lua_upvalueindex(1)); /* buffer */
  lua_pushstring(L, s);
  lua_rawseti(L, -2, luaL_len(L, -2) + 1);
  lua_pop(L, 1); /* buffer */
  return 0;
}

static int state_dostring (lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  if (luaL_dostring(L, s)) /* error? */
    lua_error(L); /* propagate */
  else {
    luaL_Buffer b;
    int i, n;
    /* collect results */
    n = lua_gettop(L);
    if (n > 1) { /* any results? */
      lua_getglobal(L, "print");
      lua_replace(L, 1);
      lua_call(L, n - 1, 0);
    }
    /* collect buffer */
    luaL_buffinit(L, &b);
    lua_pushvalue(L, lua_upvalueindex(1)); /* buffer table */
    n = luaL_len(L, -1);
    for (i = 1; i <= n; i++) {
      lua_rawgeti(L, -1, i); /* buf[i] */
      s = lua_tostring(L, -1);
      if (i > 1) luaL_addchar(&b, '\n');
      luaL_addlstring(&b, s, strlen(s));
      lua_pop(L, 1);
      lua_pushnil(L);
      lua_rawseti(L, -2, i); /* buf[i] = nil */
    }
    luaL_pushresult(&b);
  }
  return 1;
}


/* [ API ] */

static const luaL_Reg numlua_libs[] = {
#if LUA_VERSION_NUM <= 501
  {"", luaopen_base},
#else
  {"_G", luaopen_base},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_BITLIBNAME, luaopen_bit32},
#endif
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  /* FIXME: sandbox */
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_LOADLIBNAME, luaopen_package},
  {NULL, NULL}
};

lua_State *nlua_newstate () {
  const luaL_Reg *reg = numlua_libs;
  lua_State *L = luaL_newstate();
  for (; reg->func; reg++) {
#if LUA_VERSION_NUM <= 501
    lua_pushcfunction(L, reg->func);
    lua_pushstring(L, reg->name);
    lua_call(L, 1, 0);
#else
    luaL_requiref(L, reg->name, reg->func, 1); /* set in global table */
    lua_pop(L, 1); /* remove lib */
#endif
  }
#ifndef NLWS_NO_NUMLUA
  /* require numlua */
  lua_getglobal(L, "require");
  lua_pushvalue(L, -1);
  lua_pushliteral(L, "luarocks.loader");
  lua_call(L, 1, 0);
  lua_pushliteral(L, "numlua.seeall");
  lua_call(L, 1, 0);
  /* TODO: plots */
#endif
  /* new "print" and "dostring" */
  lua_pushlightuserdata(L, (void *) L);
  lua_newtable(L); /* msg buffer */
  lua_pushvalue(L, -1);
  lua_pushcclosure(L, state_print, 1);
  lua_setglobal(L, "print");
  lua_pushcclosure(L, state_dostring, 1);
  lua_rawset(L, LUA_REGISTRYINDEX); /* REG[light(L)] = dostring */
  return L;
}

/* leaves result (string) on top of stack; returns call status */
int nlua_dostring (lua_State *L, char *s, size_t l) {
  lua_pushlightuserdata(L, (void *) L);
  lua_rawget(L, LUA_REGISTRYINDEX); /* dostring */
  if (s[0] == '=')
    lua_pushfstring(L, "return %s", s + 1);
  else
    lua_pushlstring(L, s, l);
  return lua_pcall(L, 1, 1, 0);
}

