#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>

#include "luawrapperutil.h"

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

	inline void clean() {
		int n = lua_gettop(L);
		lua_pop(L, n);
	}

	std::vector<int> getIntVector(const std::string& name);
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

template<>
inline bool LuaScript::lua_get(const std::string& variableName) {
	return (bool)lua_toboolean(L, -1);
}

template<>
inline float LuaScript::lua_get(const std::string& variableName) {
	if (!lua_isnumber(L, -1)) {
		printError(variableName, "Not a number");
	}

	return (float)lua_tonumber(L, -1);
}

template <>
inline int LuaScript::lua_get(const std::string& variableName) {
    if(!lua_isnumber(L, -1)) {
      printError(variableName, "Not a number");
    }
    return (int)lua_tonumber(L, -1);
}

template <>
inline std::string LuaScript::lua_get(const std::string& variableName) {
    std::string s = "null";
    if(lua_isstring(L, -1)) {
      s = std::string(lua_tostring(L, -1));
    } else {
      printError(variableName, "Not a string");
    }
    return s;
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

std::vector<int> LuaScript::getIntVector(const std::string& name) {
	std::vector<int> v;
	lua_gettostack(name);
	if (lua_isnil(L, -1)) {
		return std::vector<int>();
	}

	lua_pushnil(L);

	while (lua_next(L, -2)) {
		v.push_back((int)lua_tonumber(L, -1));
		lua_pop(L, 1);
	}

	clean();
	return v;
}

class Character {
public:
    Character(const char* name, int hp);
    void say(const char* text);
    void heal(Character* character);
	const char* getName() { return name; }
    int getHealth() { return health; }
    void setHealth(int hp) { health = hp; }
	// will be implemented later
	void interact(Character* character);
private:
    const char* name;
    int health;
};

Character::Character(const char* name, int hp)
: name(name)
, health(hp)
{
}
 
void Character::say(const char* text) {
    std::cout << name << ":" << text << std::endl;
}
 
void Character::heal(Character* character) {
    character->setHealth(100);
}

void Character::interact(Character* character) {

}

Character* Character_new(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	int hp = luaL_checknumber(L, 2);
	return new Character(name, hp);
}

int Character_getName(lua_State* L) {
	Character* character = luaW_check<Character>(L, 1);
	lua_pushstring(L, character->getName());
	return 1;
}

int Character_getHealth(lua_State* L) {
    Character* character = luaW_check<Character>(L, 1);
    lua_pushnumber(L, character->getHealth());
    return 1;
}
 
int Character_setHealth(lua_State* L) {
    Character* character = luaW_check<Character>(L, 1);
    int hp = luaL_checknumber(L, 2);
    character->setHealth(hp);
    return 0;
}

static luaL_Reg Character_table[] = {
	{ NULL, NULL }
};

static luaL_Reg Character_metatable[] = {
	{ "getName", Character_getName },
	{ "getHealth", Character_getHealth },
	{ "setHealth", Character_setHealth },
	{ NULL, NULL }
};

static int luaopen_Character(lua_State* L) {
	luaW_register<Character>(L, "Character", Character_table, Character_metatable, Character_new);
	return 1;
}

// TODO: get rid of hard-coded path
static std::string scriptsPath = "C:\\Users\\mbradberry\\dev\\lumos\\Lumos\\Lumos\\scripts\\";

int main (void) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_Character(L);

	std::string script = scriptsPath + "test.lua";

	if (luaL_loadfile(L, script.c_str())) {
		std::cout << "couldn't open script" << std::endl;
	}

	lua_pcall(L, 0, 0, 0);

	getchar();
	return 0;
}