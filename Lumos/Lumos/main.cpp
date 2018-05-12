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
	T get(const std::string& variableName);
	bool lua_gettostack(const std::string& variableName);
 
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
	int level;
};

LuaScript::LuaScript(const std::string& filename) 
: level(0)
{
    L = luaL_newstate();
	luaL_openlibs(L); /* opens the standard libraries */
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
inline std::string LuaScript::lua_getdefault(const std::string& variableName) {
	return "null";
}

template <typename T>
T LuaScript::get(const std::string& variableName) {
	if (!L) {
		printError(variableName, "Script is not loaded!");
		return lua_getdefault<T>();
	}

	T result;
	if (lua_gettostack(variableName)) {
		result = lua_get<T>(variableName);
	}
	else {
		result = lua_getdefault<T>();
	}

	lua_pop(L, level + 1);
	return result;
}

bool LuaScript::lua_gettostack(const std::string& variableName) {
	level = 0;
	std::string var = "";
	for (int i = 0; i < variableName.size(); ++i) {
		if (variableName.at(i) == '.') {
			if (level == 0) {
				lua_getglobal(L, var.c_str());
			}
			else {
				lua_getfield(L, -1, var.c_str());
			}

			if (lua_isnil(L, -1)) {
				printError(variableName, var + " is not defined!");
				return false;
			}
			else {
				var += variableName.at(i);
			}
		}
	}

	if (level == 0) {
		lua_getglobal(L, var.c_str());
	}
	else {
		lua_getfield(L, -1, var.c_str());
	}

	if (lua_isnil(L, -1)) {
		printError(variableName, var + " is not defined!");
		return false;
	}

	return true;
}

// TODO: get rid of hard-coded path
static std::string scriptsPath = "C:\\Users\\mbradberry\\dev\\lumos\\Lumos\\Lumos\\scripts\\";

int main (void) {
	std::string script = scriptsPath + "test.lua";
	LuaScript ls(script);

	getchar();
	return 0;
}