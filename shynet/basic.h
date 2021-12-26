#ifndef SHYNET_BASIC_H
#define SHYNET_BASIC_H

#include "3rd/backward/backward.hpp"
#include <cstring>
#include <string>

// 无锁数据成员
#define PROPERTY(Type, name)                 \
public:                                      \
    Type name() const { return name##_; }    \
    void set_##name(Type v) { name##_ = v; } \
                                             \
private:                                     \
    Type name##_;

// 带初始化值的数据成员
#define PROPERTY_WITH_INIT_VALUE(Type, name, value) \
public:                                             \
    Type name() const { return name##_; }           \
    void set_##name(Type v) { name##_ = v; }        \
                                                    \
private:                                            \
    Type name##_ = value;

// 锁数据成员
#define PROPERTY_LOCK(LockType, lockName) \
private:                                  \
    LockType lockName##_;

// 带锁的数据成员
#define PROPERTY_WITH_LOCK(GuardType, LockType, lockName, PropertyType, propertyName) \
public:                                                                               \
    PropertyType propertyName()                                                       \
    {                                                                                 \
        GuardType<LockType> l(lockName##_);                                           \
        return propertyName##_;                                                       \
    }                                                                                 \
    void set_##propertyName(PropertyType v) { propertyName##_ = v; }                  \
                                                                                      \
private:                                                                              \
    PropertyType propertyName##_;

// c++11锁
#define PROPERTY_STD_LOCK(name) PROPERTY_LOCK(std::mutex, name)
#define PROPERTY_WITH_STD_LOCK(lockName, PropertyType, propertyName) PROPERTY_WITH_LOCK(std::lock_guard, std::mutex, lockName, PropertyType, propertyName)

#define THROW_EXCEPTION(err)                                                      \
    {                                                                             \
        std::ostringstream oss;                                                   \
        oss << err << " -" << strrchr(__FILE__, '/') + 1 << " line:" << __LINE__; \
        std::throw_with_nested(shynet::BaseException(oss.str()));                 \
    }

namespace shynet {
namespace utils {
};
namespace events {
};
namespace net {
    /**
         * @brief 连接断开原因
        */
    enum class CloseType {
        /**
         * @brief 客户端主动断开
            */
        CLIENT_CLOSE,
        /**
         * @brief 服务区主动断开
        */
        SERVER_CLOSE,
        /**
             * @brief 连接服务器失败
            */
        CONNECT_FAIL,
        /**
             * @brief 心跳包超时
            */
        TIMEOUT_CLOSE,
    };

    /**
     * @brief 管理io缓冲数据处理结果
    */
    enum class InputResult {
        SUCCESS = 0,
        /**
         * @brief 主动断开
        */
        INITIATIVE_CLOSE,
        /**
         * @brief 被动断开
        */
        PASSIVE_CLOSE,
        /**
         * @brief 数据包不完整
        */
        DATA_INCOMPLETE,
    };
};
namespace io {
};
namespace signal {
};
namespace pool {
};
namespace crypto {
};
namespace thread {
};
namespace lua {
};
namespace task {
};
namespace protocol {
    /**
     * @brief 协议处理步骤阶段
    */
    enum class Step {
        /**
         * @brief 未初始阶段
        */
        UNINIT = 0,
        /**
         * @brief 已初始阶段
        */
        INIT,
        /**
         * @brief 解析阶段
        */
        Parse,
    };
};

/**
 * @brief 异常
*/
class BaseException : public std::exception {
public:
    explicit BaseException(const std::string msg)
        : msg_(msg)
    {
        backward::StackTrace st;
        st.load_here(32);
        backward::Printer p;
        std::ostringstream trace;
        p.object = true;
        p.address = true;
        p.print(st, trace);
        trace_ = trace.str();
    }

    BaseException(const BaseException&) = default;
    BaseException& operator=(const BaseException&) = default;

    BaseException(BaseException&&) = default;
    BaseException& operator=(BaseException&&) = default;

    virtual ~BaseException() override = default;

    //异常信息
    virtual const char* what() const noexcept override
    {
        return msg_.data();
    }

    //堆栈追踪
    const char* trace() const noexcept
    {
        return trace_.c_str();
    }

private:
    std::string msg_;
    std::string trace_;
};

/**
 * @brief 禁止拷贝
*/
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
namespace io = shynet::io;
namespace signals = shynet::signal;

#endif
