#pragma once
#include "frmpub/FrmLuaWrapper.h"

namespace dbvisit {
	class DbLuaWrapper : public frmpub::FrmLuaWrapper {
	public:
		DbLuaWrapper();
		~DbLuaWrapper();

		virtual void init(kaguya::State& state) override;
	};
}
