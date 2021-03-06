#include "shynet/lua/luaengine.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"

namespace shynet {
namespace lua {
    LuaEngine::LuaEngine(std::shared_ptr<LuaWrapper> wrapper)
    {
        wrapper_ = wrapper;
    }

    void LuaEngine::init(kaguya::State& state)
    {
        if (wrapper_) {
            wrapper_->init(state);
        }
    }

    void LuaEngine::append(std::shared_ptr<luatask::LuaTask> task)
    {
        auto ptr = utils::Singleton<pool::ThreadPool>::instance().luaTh().lock();
        if (ptr) {
            ptr->addTask(task);
        }
    }

}
}
