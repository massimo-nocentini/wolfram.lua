
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

    lp = WSOpenString(ep, "-linkmode launch -linkname '/usr/local/Wolfram/WolframEngine/13.2/Executables/wolfram -wstp -noprompt -noicon'", &err);

    if (lp == (WSLINK)0 || err != WSEOK)
    {
        luaL_error(L, "unable to create link to the Kernel: %d", err);
    }

    WSActivate(lp);
    if (!WSActivate(lp))
    {
        luaL_error(L, "unable to establish communication");
    }

    WSPutFunction(lp, "EvaluatePacket", 1L);
    WSPutFunction(lp, "FactorInteger", 1L);
    WSPutInteger(lp, n);
    WSEndPacket(lp);
    WSFlush(lp);

    printf("Sent\n");

    while ((pkt = WSNextPacket(lp), pkt) && pkt != RETURNPKT)
    {
        printf(".\n");
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

void rec(lua_State *L, WSLINK lp)
{
    const char *string;
    int bytes;
    int characters;
    int n;

    switch (WSGetNext(lp))
    {
    case WSTKFUNC:

        if (!WSGetUTF8Function(lp, &string, &characters, &n))
        {
            luaL_error(L, "unable to read the function from lp");
        }

        lua_createtable(L, 0, 2);

        lua_pushstring(L, string);
        lua_setfield(L, -2, "head");

        WSReleaseUTF8Symbol(lp, string, characters);

        lua_createtable(L, n, 0);
        for (int i = 0; i < n; i++)
        {
            rec(L, lp);
            lua_seti(L, -2, i + 1);
        }

        lua_setfield(L, -2, "arguments");

        break;

    case WSTKSTR:

        if (!WSGetUTF8String(lp, &string, &bytes, &characters))
        {
            luaL_error(L, "unable to read the UTF-8 string from lp");
        }

        lua_pushstring(L, string);

        WSReleaseUTF8String(lp, string, bytes);

        break;

    case WSTKSYM:

        if (!WSGetUTF8Symbol(lp, &string, &bytes, &characters))
        {
            luaL_error(L, "unable to read the UTF-8 symbol from lp");
        }

        lua_createtable(L, 0, 2);

        lua_pushstring(L, "__symbol");
        lua_setfield(L, -2, "head");

        lua_createtable(L, 1, 0);
        lua_pushstring(L, string);
        lua_seti(L, -2, 1);

        lua_setfield(L, -2, "arguments");

        WSReleaseUTF8Symbol(lp, string, bytes);

        break;

    case WSTKINT:

        wsint64 i;
        if (!WSGetInteger64(lp, &i))
        {
            luaL_error(L, "unable to read the long from lp");
        }

        lua_pushinteger(L, i);

        break;

    case WSTKREAL:

        double r;
        if (!WSGetReal64(lp, &r))
        {
            luaL_error(L, "unable to read the real from lp");
        }

        lua_pushnumber(L, r);

        break;

    default:
        luaL_error(L, "Unhandled value of type id %d.", WSGetType(lp));
        break;
    }
}

int l_evaluate(lua_State *L)
{
    WSLINK lp = (WSLINK)lua_touserdata(L, 1);

    int pkt, err;
    int code, param;

    const unsigned char *string;
    int bytes;
    int characters;

    while ((pkt = WSNextPacket(lp), pkt) && pkt != RETURNPKT)
    {
        switch (pkt)
        {
        case MESSAGEPKT:
            if (!WSGetMessage(lp, &code, &param))
            {
                luaL_error(L, "unable to read the message code from lp");
            }
            printf("Got message code %d with param %d\n", code, param);
            break;
        case TEXTPKT:

            if (!WSGetUTF8String(lp, &string, &bytes, &characters))
            {
                luaL_error(L, "unable to read the UTF-8 string from lp");
            }

            printf("Got the text: %s\n", string);

            WSReleaseUTF8String(lp, string, bytes);
            break;
        default:
            printf("Got packet of type %d.\n", pkt);
            break;
        }

        WSNewPacket(lp);
        if (err = WSError(lp), err)
            luaL_error(L, "Error %d", err);
    }

    rec(L, lp);

    return 1;
}

const struct luaL_Reg libwolframlua[] = {
    {"wolfram", l_wolfram},
    {"evaluate", l_evaluate},
    {NULL, NULL} /* sentinel */
};

extern int luaopen_libwolframlua(lua_State *L)
{
    luaL_newlib(L, libwolframlua);

    return 1;
}
