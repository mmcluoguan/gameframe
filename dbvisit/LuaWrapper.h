#pragma once
#include "frmpub/FrmLuaWrapper.h"

namespace dbvisit {
	class LuaWrapper : public frmpub::FrmLuaWrapper {
	public:
		LuaWrapper();
		~LuaWrapper();

		virtual void init(kaguya::State& state) override;
	};
}
