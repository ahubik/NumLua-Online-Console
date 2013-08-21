/*
 * Numlua state websocket interface
 * Author: Luis Carvalho <lexcarvalho at gmail.com>
 * See copyright in nlstate.h
*/

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <libwebsockets.h>
#include "nlstate.h"

/* TODO: move params to config file */
#define WAIT 50 /* waiting time after processing protocol callbacks, in ms */
#define PORT 6582

static lua_State *getwsistate (lua_State *P, struct libwebsocket *wsi) {
  lua_State *L;
  lua_pushlightuserdata(P, (void *) wsi);
  lua_rawget(P, LUA_REGISTRYINDEX);
  L = (lua_State *) lua_touserdata(P, -1);
  lua_pop(P, 1); /* balance stack */
  return L;
}

/* [ callback ] */
static int repl_callback (struct libwebsocket_context *ctx,
    struct libwebsocket *wsi,
    enum libwebsocket_callback_reasons reason,
    void *user, void *in, size_t len)
{
  lua_State *P = (lua_State *) libwebsocket_context_user(ctx);
  (void) user; /* avoid warnings */
  switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
      lua_pushlightuserdata(P, (void *) wsi);
      lua_pushlightuserdata(P, (void *) nlua_newstate());
      lua_rawset(P, LUA_REGISTRYINDEX);
      fprintf(stderr, "[LOG] Connection established\n");
      break;
    case LWS_CALLBACK_CLOSED:
      lua_close(getwsistate(P, wsi));
      fprintf(stderr, "[LOG] Connection closed\n");
      break;
    case LWS_CALLBACK_RECEIVE: {
      lua_State *L = getwsistate(P, wsi);
      int status;
      luaL_Buffer b;
      unsigned char *buf;
      const char *result;
      size_t rlen;
      /* process request */
      status = nlua_dostring(L, (char *) in, len);
      /* prepare output */
      luaL_buffinit(P, &b);
      buf = (unsigned char *) luaL_prepbuffer(&b);
      luaL_addsize(&b, LWS_SEND_BUFFER_PRE_PADDING); /* pre-pad */
      result = lua_tostring(L, lua_gettop(L));
      rlen = strlen(result);
      luaL_addchar(&b, status); /* status in first byte (header) */
      luaL_addlstring(&b, result, rlen);
      luaL_addsize(&b, LWS_SEND_BUFFER_POST_PADDING); /* post-pad */
      lua_pop(L, 1); /* remove result from 'dostring' in L */
      /* push result */
      libwebsocket_write(wsi, buf + LWS_SEND_BUFFER_PRE_PADDING, rlen + 1,
          LWS_WRITE_TEXT);
      break;
    }
    default:
      fprintf(stderr, "[LOG] Unhandled protocol: %d\n", reason);
      break;
  }
  return 0;
}

static struct libwebsocket_protocols protocols[] = {
  /* note: first protocol must always be HTTP handler */
  { "repl", repl_callback, 0, 0, NULL, 0}, /* TODO: per session data: one Lua state */
  { NULL, NULL, 0, 0, NULL, 0}
};


int main (void) {
  /* TODO: initialize with LuaMacro */
  lua_State *L = luaL_newstate(); /* user */
  struct lws_context_creation_info info;
  struct libwebsocket_context *context;

  /* setup info */
  memset(&info, 0, sizeof(info));
  info.port = PORT;
  info.iface = NULL;
  info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
  info.extensions = libwebsockets_get_internal_extensions();
#endif
  info.ssl_cert_filepath = NULL; /* TODO: not using SSL for now */
  info.ssl_private_key_filepath = NULL; /* TODO */
  info.gid = -1;
  info.uid = -1;
  info.options = 0;
  info.user = (void *) L;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    fprintf(stderr, "libwebsocket failed to initialize\n");
    return 1;
  }
  fprintf(stderr, "[LOG] Starting server\n");

  for (;;) libwebsocket_service(context, WAIT);
  libwebsocket_context_destroy(context);
  return 0;
}

