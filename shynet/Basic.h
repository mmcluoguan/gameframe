#pragma once
#include <string>
#include <cstring>

// 无锁数据成员
#define PROPERTY(Type, name) \
  public: \
    Type name() const { return name##_; } \
    void set_##name(Type v) { name##_ = v; } \
  private: \
    Type name##_;

// 带初始化值的数据成员
#define PROPERTY_WITH_INIT_VALUE(Type, name, value) \
  public: \
    Type name() const { return name##_; } \
    void set_##name(Type v) { name##_ = v; } \
  private: \
    Type name##_ = value;

// 锁数据成员
#define PROPERTY_LOCK(LockType, lockName) \
  private: \
    LockType lockName##_;

// 带锁的数据成员
#define PROPERTY_WITH_LOCK(GuardType, LockType, lockName, PropertyType, propertyName) \
  public: \
    PropertyType propertyName() { GuardType<LockType> l(lockName##_); return propertyName##_; } \
    void set_##propertyName(PropertyType v) { propertyName##_ = v; } \
  private: \
    PropertyType propertyName##_;

// c++11锁
#define PROPERTY_STD_LOCK(name) PROPERTY_LOCK(std::mutex, name)
#define PROPERTY_WITH_STD_LOCK(lockName, PropertyType, propertyName) PROPERTY_WITH_LOCK(std::lock_guard, std::mutex, lockName, PropertyType, propertyName)

#define THROW_EXCEPTION(err)  std::throw_with_nested(shynet::BaseException(std::string(err) + " -" + std::string(strrchr(__FILE__, '/')+1) + " line:" + std::to_string(__LINE__)));

namespace shynet {
	namespace utils {};
	namespace events {};
	namespace net {};
	namespace pool {};
	namespace crypto {};
	namespace thread {};
	namespace lua {};
	namespace task {};
	namespace protocol {
		enum class Step {
			UNINIT = 0,
			INIT,
			Parse,
		};
	};

	/*
	* 异常
	*/
	class BaseException : public std::exception {
	public:
		explicit BaseException(const std::string& msg) : msg_(msg) {}

		BaseException(const BaseException&) = default;
		BaseException& operator=(const BaseException&) = default;

		BaseException(BaseException&&) = default;
		BaseException& operator=(BaseException&&) = default;

		virtual ~BaseException() override = default;

		virtual const char* what() const noexcept override {
			return msg_.data();
		}

	private:
		std::string msg_;
	};

	/// <summary>
	/// 禁止拷贝
	/// </summary>
	class Nocopy {
	public:
		Nocopy(const Nocopy&) = delete;
		Nocopy(Nocopy&) = delete;
		Nocopy(Nocopy&&) = delete;
		Nocopy& operator=(const Nocopy&) = delete;
		Nocopy& operator=(Nocopy&) = delete;
		Nocopy& operator=(Nocopy&&) = delete;
		virtual ~Nocopy() = default;
	protected:
		Nocopy() = default;
	};
}

namespace events = shynet::events;
namespace net = shynet::net;
namespace pool = shynet::pool;
namespace crypto = shynet::crypto;
namespace protocol = shynet::protocol;
namespace thread = shynet::thread;
namespace lua = shynet::lua;
namespace task = shynet::task;
