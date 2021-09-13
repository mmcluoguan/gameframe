#pragma once
#include "frmpub/FrmLuaWrapper.h"

namespace world {
	class LuaWrapper : public frmpub::FrmLuaWrapper {
	public:
		LuaWrapper();
		~LuaWrapper();

		virtual void init(kaguya::State& state) override;
	};
}
