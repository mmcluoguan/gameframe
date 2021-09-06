#pragma once
#include "shynet/kaguya/kaguya.hpp"
#include "shynet/Singleton.h"
#include "shynet/Utility.h"

namespace frmpub {
	/*
	* lua远程调试
	*/
	class LuaRemoteDebug final : public shynet::Nocopy {
		friend class shynet::Singleton<LuaRemoteDebug>;

		LuaRemoteDebug() = default;
	public:
		/*
		* 初始化调试地址
		*/
		LuaRemoteDebug& init(const std::string debugip);

		/*
		* 开始调试
		*/
		LuaRemoteDebug& start(kaguya::State& state);

		/*
		* 调试结束
		*/
		LuaRemoteDebug& stop(kaguya::State& state);
	private:
		std::string debugip_;
		bool isinit_ = false;
	};
}
