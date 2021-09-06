#pragma once
#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"
#include "frmpub/protocc/common.pb.h"
#include "dbvisit/DbClient.h"

namespace dbvisit {
	class OnAcceptTask : public shynet::task::Task {
	public:
		OnAcceptTask(std::shared_ptr<DbClient> dbclient);
		~OnAcceptTask();

		virtual int run(thread::Thread* tif);
	private:
		std::shared_ptr<DbClient> dbclient_;
	};

	class OnCloseTask : public shynet::task::Task {
	public:
		OnCloseTask(int fd);
		~OnCloseTask();

		virtual int run(thread::Thread* tif);
	private:
		int fd_;
	};

	class OnMessageTask : public shynet::task::Task {
	public:
		OnMessageTask(std::shared_ptr<DbClient> dbclient, std::shared_ptr<protocc::CommonObject> data);
		~OnMessageTask();

		virtual int run(thread::Thread* tif);
	private:
		std::shared_ptr<protocc::CommonObject> data_;
		std::shared_ptr<DbClient> dbclient_;
	};
}
