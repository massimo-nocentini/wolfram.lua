
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <lua.h>
#include <lauxlib.h>

#include <wstp.h>

int l_wolfram(lua_State *L)
{
    WSENV ep;
    WSLINK lp;

    int pkt, n, prime, expt, err;
    int len, lenp, k;

    n = lua_tointeger(L, 1);

    ep = WSInitialize((WSEnvironmentParameter)0);
    if (ep == (WSENV)0)
    {
        luaL_error(L, "unable to initialize environment");
    }

    lp = WSOpenString(ep, "lua -linkname '/usr/local/Wolfram/WolframEngine/13.2/Executables/math -wstp'", &err);

    if (lp == (WSLINK)0 || err != WSEOK)
    {
        luaL_error(L, "unable to create link to the Kernel: %d", err);
    }

    WSPutFunction(lp, "EvaluatePacket", 1L);
    WSPutFunction(lp, "FactorInteger", 1L);
    WSPutInteger(lp, n);
    WSEndPacket(lp);

    printf("Sent\n");

    while ((pkt = WSNextPacket(lp), pkt) && pkt != RETURNPKT)
    {
        WSNewPacket(lp);
        if (WSError(lp))
            luaL_error(L, "a");
    }

    printf("Received\n");

    if (!WSTestHead(lp, "List", &len))
        luaL_error(L, "b");

    lua_newtable(L);
    for (k = 1; k <= len; k++)
    {
        if (WSTestHead(lp, "List", &lenp) && lenp == 2 && WSGetInteger(lp, &prime) && WSGetInteger(lp, &expt))
        {

            lua_newtable(L);

            lua_pushinteger(L, prime);
            lua_setfield(L, -2, "prime");

            lua_pushinteger(L, expt);
            lua_setfield(L, -2, "expt");

            lua_seti(L, -2, k);

            // printf("%d ^ %d\n", prime, expt);
        }
        else
        {
            luaL_error(L, "c");
        }
    }

    WSPutFunction(lp, "Exit", 0);

    WSClose(lp);
    WSDeinitialize(ep);
    return 1;
}

const struct luaL_Reg libwolframlua[] = {
    {"wolfram", l_wolfram},
    {NULL, NULL} /* sentinel */
};

extern int luaopen_libwolframlua(lua_State *L)
{
    luaL_newlib(L, libwolframlua);

    return 1;
}
