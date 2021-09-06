#pragma once
#include "shynet/fflua/fflua.h"

namespace client {
	class LuaWrapper {
	public:
		static void init(ff::fflua_t& fflua);
	};
}
