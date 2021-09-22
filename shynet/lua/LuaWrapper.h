#ifndef SHYNET_LUA_LUAWRAPPER_H
#define SHYNET_LUA_LUAWRAPPER_H

#include <kaguya/kaguya.hpp>

namespace shynet {
	namespace lua {
		/*
		* 导出c++到lua
		*/
		class LuaWrapper {
		public:
			LuaWrapper();
			virtual ~LuaWrapper();

			/*
			* 初始化lua状态
			*/
			virtual void init(kaguya::State& state);
		};
	}
}

#endif
