#include "lprefix.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

void cydosOpenLibs(lua_State *L) {
  const luaL_Reg libs[] = { { "_G", luaopen_base },
                            { LUA_MATHLIBNAME, luaopen_math },
                            { LUA_STRLIBNAME, luaopen_string },
                            { LUA_TABLIBNAME, luaopen_table },
                            { NULL, NULL } };
  const luaL_Reg *lib;
  for (lib = libs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }
}
