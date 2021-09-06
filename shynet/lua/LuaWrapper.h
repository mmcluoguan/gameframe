#pragma once
#include "shynet/kaguya/kaguya.hpp"

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
