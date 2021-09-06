#include "dbvisit/LuaTask.h"
#include "shynet/thread/LuaThread.h"

namespace dbvisit {
	OnAcceptTask::OnAcceptTask(std::shared_ptr<DbClient> dbclient) {
		GOBJ++;
		LOG_TRACE << "OnAcceptTask:" << GOBJ;
		dbclient_ = dbclient;
	}

	OnAcceptTask::~OnAcceptTask() {
		GOBJ--;
		LOG_TRACE << "~OnAcceptTask:" << GOBJ;
	}

	int OnAcceptTask::run(thread::Thread* tif) {
		thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
		lua->luaState()->operator[]("onAccept").call<void>(dbclient_.get());
		return 0;
	}

	OnCloseTask::OnCloseTask(int fd) {
		GOBJ++;
		LOG_TRACE << "OnCloseTask:" << GOBJ;
		fd_ = fd;
	}

	OnCloseTask::~OnCloseTask() {
		GOBJ--;
		LOG_TRACE << "~OnCloseTask:" << GOBJ;
	}

	int OnCloseTask::run(thread::Thread* tif) {
		thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
		lua->luaState()->operator[]("onClose").call<void>(fd_);
		return 0;
	}

	OnMessageTask::OnMessageTask(std::shared_ptr<DbClient> dbclient, std::shared_ptr<protocc::CommonObject> data) {
		GOBJ++;
		LOG_TRACE << "OnMessageTask:" << GOBJ;
		data_ = data;
		dbclient_ = dbclient;
	}

	OnMessageTask::~OnMessageTask() {
		GOBJ--;
		LOG_TRACE << "~OnMessageTask:" << GOBJ;
	}

	int OnMessageTask::run(thread::Thread* tif) {
		thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
		if (dbclient_) {
			lua->luaState()->operator[]("onMessage").call<void>(dbclient_.get(), data_->msgid(), data_->msgdata());
		}
		return 0;
	}
}
