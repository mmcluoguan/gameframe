#ifndef SHYNET_LUA_LUAENGINE_H
#define SHYNET_LUA_LUAENGINE_H

#include "shynet/lua/LuaWrapper.h"
#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"
#include "shynet/utils/Singleton.h"

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
        void append(std::shared_ptr<task::Task> task);

    private:
        std::shared_ptr<LuaWrapper> wrapper_;
    };
}
}

#endif
