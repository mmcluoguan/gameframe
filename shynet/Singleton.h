#pragma once
#include <mutex>
#include <memory>
#include "shynet/Logger.h"
#include "shynet/Utility.h"

namespace shynet {
	template <class T>
	class Singleton : public Nocopy {
	public:
		Singleton() {
		}
		~Singleton() {
		}

		template<typename...  Args>
		static T& instance(Args&&... args) {
			std::call_once(onceflag_,
				std::forward<void(Args&&...)>(&Singleton<T>::init),
				std::forward<Args>(args)...);
			return *instance_;
		};

		static T& get_instance() {
			if (instance_ == nullptr) {
				LOG_ERROR << T::classname << " single is not initialized";
			}
			return *instance_;
		}
	private:
		template<typename...Args>
		static void init(Args&&...args) {
			instance_.reset(new T(std::forward<Args>(args)...));
		}
	private:
		static std::once_flag onceflag_;
		static std::unique_ptr<T> instance_;
	};

	template <class T>
	std::unique_ptr<T> Singleton<T>::instance_;
	template <class T>
	std::once_flag Singleton<T>::onceflag_;
}
