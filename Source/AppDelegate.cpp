/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2021 Bytedance Inc.

 https://axmolengine.github.io/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "cocos2d.h"
#include "lauxlib.h"
#include "scripting/lua-bindings/manual/lua_module_register.h"

USING_NS_AX;



extern void reupload_main(
	std::string gdpsUsername,
	std::string gdpsPassword,
	std::string gdUsername,
	std::string gdPassword,
	int levelID
);

std::string getFromLua(lua_State* L, int n)
{
	size_t l = 1;
	const char* str = luaL_checklstring(L, n, &l);
	AXLOG("parsing: %s", str);
	return {str, l};
}

static int reupload_level(lua_State* L)
{
	std::string gdpsUsername = getFromLua(L, 1);
	std::string gdpsPassword = getFromLua(L, 2);
	std::string gdUsername = getFromLua(L, 3);
	std::string gdPassword = getFromLua(L, 4);
	int levelID = luaL_checkint(L, 5);
	AXLOG("levelID: %d", levelID);
	
	reupload_main(gdpsUsername, gdpsPassword, gdUsername, gdPassword, levelID);
	return 0;
}

void callLuaResult(int result, const char* error)
{
	auto engine = LuaEngine::getInstance();
	ScriptEngineManager::getInstance()->setScriptEngine(engine);
	lua_State* L = engine->getLuaStack()->getLuaState();
	lua_getglobal(L, "reupload_callback");
	
	lua_pushnumber(L, result);
	
	if(!error)
		lua_pushnil(L);
	else
		lua_pushstring(L, error);
	
	
	lua_call(L, 2, 0);
}



#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#	include "audio/AudioEngine.h"
#endif

using namespace std;

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
	// set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
	GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

	GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,
// don't modify or remove this function
static int register_all_packages()
{
	return 0;  // flag for packages manager
}



bool AppDelegate::applicationDidFinishLaunching()
{
	// set default FPS
	Director::getInstance()->setAnimationInterval(1.0 / 60.0f);

	// register lua module
	auto engine = LuaEngine::getInstance();
	ScriptEngineManager::getInstance()->setScriptEngine(engine);
	lua_State* L = engine->getLuaStack()->getLuaState();
	lua_module_register(L);
	register_all_packages();

	lua_pushcfunction(L, reupload_level);
	lua_setglobal(L, "reupload_level");

	LuaStack* stack = engine->getLuaStack();

	// register custom function
	// LuaStack* stack = engine->getLuaStack();
	// register_custom_function(stack->getLuaState());

	stack->addSearchPath("src");
	FileUtils::getInstance()->addSearchPath("res");
	if (engine->executeString("require 'main'"))
	{
		return false;
	}

	return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground()
{
	Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
	Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::resumeAll();
#endif
}
