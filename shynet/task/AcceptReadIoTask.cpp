#include "shynet/task/AcceptReadIoTask.h"

namespace shynet {
	namespace task {
		AcceptReadIoTask::AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd) {
			newfd_ = newfd;
		}
		AcceptReadIoTask::~AcceptReadIoTask() {
		}

		int AcceptReadIoTask::run(thread::Thread* tif) {
			std::shared_ptr<net::AcceptNewFd> aptnewfd = newfd_.lock();
			if (aptnewfd != nullptr) {
				std::shared_ptr<net::AcceptHeartbeat> ht = aptnewfd->heart().lock();
				if (ht != nullptr) {
					ht->val({ aptnewfd->heart_second(),0 });
				}
				int ret = aptnewfd->input();
				if (ret == -1) {
					aptnewfd->close(true);
				}
				else if (ret == -2) {
					aptnewfd->close(false);
				}
			}
			return 0;
		}
	}
}
