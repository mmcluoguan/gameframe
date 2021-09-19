#ifndef LOGIN_LUAWRAPPER_H
#define LOGIN_LUAWRAPPER_H

#include "frmpub/FrmLuaWrapper.h"

namespace login {
	class LuaWrapper : public frmpub::FrmLuaWrapper {
	public:
		LuaWrapper();
		~LuaWrapper();

		virtual void init(kaguya::State& state) override;
	};
}

#endif
