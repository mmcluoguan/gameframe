#pragma once
#include <shynet/thread/Thread.h>

namespace shynet {
	namespace task {
		class Task : public Nocopy {
		public:
			Task();
			~Task();

			virtual int run(thread::Thread* tif) = 0;
		};
	}
}
