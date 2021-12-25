#ifndef SHYNET_LUA_LUAENGINE_H
#define SHYNET_LUA_LUAENGINE_H

#include "shynet/lua/luatask.h"
#include "shynet/lua/luawrapper.h"
#include "shynet/thread/thread.h"
#include "shynet/utils/singleton.h"

namespace shynet {
namespace lua {
    /// <summary>
    /// lua通信引擎
    /// </summary>
    class LuaEngine final : public Nocopy {
        friend class utils::Singleton<LuaEngine>;

        LuaEngine(std::shared_ptr<LuaWrapper> wrapper = nullptr);

    public:
        static constexpr const char* kClassname = "LuaEngine";
        ~LuaEngine();

        /*
			* 初始化lua栈
			*/
        void init(kaguya::State& state);

        /*
			* 添加任务到lua线程
			*/
        void append(std::shared_ptr<luatask::LuaTask> task);

    private:
        std::shared_ptr<LuaWrapper> wrapper_;
    };
}
}

#endif
