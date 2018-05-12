#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

using namespace std;

class LuaScript {
public:
    LuaScript(const std::string& filename);
    ~LuaScript();
    void printError(const std::string& variableName, const std::string& reason);
 
    template<typename T>
    T get(const std::string& variableName) {
        // will be implemented later in tutorial
    }
    bool lua_gettostack(const std::string& variableName) {
       // will be explained later too
    }
 
    // Generic get
    template<typename T>
    T lua_get(const std::string& variableName) {
      return 0;
    }
 
    // Generic default get
    template<typename T>
    T lua_getdefault(const std::string& variableName) {
      return 0;
    }
private:
    lua_State* L;
};

LuaScript::LuaScript(const std::string& filename) {
    L = luaL_newstate();
    if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, 0, 0)) {
        std::cout<<"Error: script not loaded ("<<filename<<")"<<std::endl;
        L = 0;
    }
}

LuaScript::~LuaScript() {
    if(L) lua_close(L);
}

void LuaScript::printError(const std::string& variableName, const std::string& reason) {
    std::cout<<"Error: can't get ["<<variableName<<"]. "<<reason<<std::endl;
}

template<>
inline std::string LuaScript::lua_getdefault() {
	return "null";
}

// TODO: get rid of hard-coded path
static std::string scriptsPath = "C:\\Users\\mbradberry\\dev\\lumos\\Lumos\\Lumos\\scripts";

int main (void) {
	char buff[256];
	int error;
	lua_State *L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */

	std::string testFile = scriptsPath + "\\test.lua";
	error = luaL_loadfile(L, testFile.c_str());
	lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s", lua_tostring(L, -1));
		lua_pop(L, 1); /* pop error message from the stack */
	}

	lua_close(L);

	getchar();
	return 0;
}