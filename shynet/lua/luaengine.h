#ifndef SHYNET_LUA_LUAENGINE_H
#define SHYNET_LUA_LUAENGINE_H

#include "shynet/lua/luatask.h"
#include "shynet/lua/luawrapper.h"
#include "shynet/thread/thread.h"
#include "shynet/utils/singleton.h"

namespace shynet {
namespace lua {
    /**
     * @brief lua通信引擎
    */
    class LuaEngine final : public Nocopy {
        friend class utils::Singleton<LuaEngine>;

        /**
         * @brief 构造
         * @param wrapper lua包装器
        */
        LuaEngine(std::shared_ptr<LuaWrapper> wrapper = nullptr);

    public:
        /**
         * @brief 类型名
        */
        static constexpr const char* kClassname = "LuaEngine";
        ~LuaEngine() = default;

        /**
         * @brief 初始化lua栈
         * @param state lua栈
        */
        void init(kaguya::State& state);

        /**
         * @brief 添加任务到lua线程
         * @param task 任务
        */
        void append(std::shared_ptr<luatask::LuaTask> task);

    private:
        /**
         * @brief lua包装器
        */
        std::shared_ptr<LuaWrapper> wrapper_;
    };
}
}

#endif
