#include "shynet/lua/LuaEngine.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/utils/Logger.h"

namespace shynet {
	namespace lua {
		LuaEngine::LuaEngine(std::shared_ptr<LuaWrapper> wrapper) {
			wrapper_ = wrapper;
		}
		LuaEngine::~LuaEngine() {
		}

		void LuaEngine::init(kaguya::State& state) {
			if (wrapper_) {
				wrapper_->init(state);
			}
		}

		void LuaEngine::append(std::shared_ptr<task::Task> task) {
			auto ptr = utils::Singleton<pool::ThreadPool>::get_instance().luaTh().lock();
			if (ptr) {
				ptr->addTask(task);
			}
		}

	}
}
