#ifndef FRMPUB_ONACCEPTTASK_H
#define FRMPUB_ONACCEPTTASK_H

#include "3rd/fmt/format.h"
#include "frmpub/luaremotedebug.h"
#include "shynet/lua/luatask.h"
#include "shynet/thread/luathread.h"
#include "shynet/thread/thread.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
/**
 * @brief 处理新连接lua任务
 * @tparam T 新连接类型
*/
template <class T>
class OnAcceptTask : public shynet::luatask::LuaTask {
public:
    /**
     * @brief 构造
     * @param client 新连接
    */
    OnAcceptTask(std::shared_ptr<T> client)
    {
        client_ = client;
    }
    ~OnAcceptTask() = default;

    /**
    * @brief 运行lua任务
    * @param tif 线程信息
    * @return 0成功,-1失败,返回失败lua线程会结束
    */
    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (client_) {
            auto cb = [&]() {
                shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
                state["onAccept"].call<void>(client_.get());
                shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
            };
#ifdef USE_DEBUG
            shynet::utils::elapsed("lua线程单任务执行 onAccept");
            cb();
#elif
            cb();
#endif
        }
        return 0;
    }

private:
    /**
     * @brief 新连接
    */
    std::shared_ptr<T> client_;
};

/**
 * @brief 处理连接服务器成功lua任务
 * @tparam T 连接服务器类型
*/
template <class T>
class OnConnectorTask : public shynet::luatask::LuaTask {
public:
    /**
     * @brief 构造
     * @param conncetor 连接服务器的连接器 
    */
    OnConnectorTask(std::shared_ptr<T> conncetor)
    {
        conncetor_ = conncetor;
    }
    ~OnConnectorTask() = default;

    /**
    * @brief 运行lua任务
    * @param tif 线程信息
    * @return 0成功,-1失败,返回失败lua线程会结束
    */
    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (conncetor_) {
            auto cb = [&]() {
                shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
                state["onConnect"].call<void>(conncetor_.get());
                shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
            };
#ifdef USE_DEBUG
            shynet::utils::elapsed("lua线程单任务执行 onConnect");
            cb();
#elif
            cb();
#endif
        }
        return 0;
    }

private:
    /**
     * @brief 连接服务器的连接器
    */
    std::shared_ptr<T> conncetor_;
};

/**
 * @brief 连接断开lua任务
*/
class OnCloseTask : public shynet::luatask::LuaTask {
public:
    /**
     * @brief 构造
     * @param fd socket文件描述符
    */
    OnCloseTask(int fd)
    {
        fd_ = fd;
    }
    ~OnCloseTask() = default;

    /**
    * @brief 运行lua任务
    * @param tif 线程信息
    * @return 0成功,-1失败,返回失败lua线程会结束
    */
    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        auto cb = [&]() {
            shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
            state["onClose"].call<void>(fd_);
            shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        };
#ifdef USE_DEBUG
        shynet::utils::elapsed("lua线程单任务执行 onClose");
        cb();
#elif
        cb();
#endif
        return 0;
    }

private:
    /**
     * @brief socket文件描述符
    */
    int fd_;
};

/**
 * @brief 处理连接数据
 * @tparam T 
*/
template <class T>
class OnMessageTask : public shynet::luatask::LuaTask {
public:
    /**
     * @brief 构造
     * @param client 对端连接
     * @param data protocbuf数据
     * @param enves 路由信息
    */
    OnMessageTask(std::shared_ptr<T> client, std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)
    {
        client_ = client;
        data_ = data;
        enves_ = enves;
    }
    /**
     * @brief 构造
     * @param client 对端连接
     * @param doc json数据
     * @param enves 路由信息
    */
    OnMessageTask(std::shared_ptr<T> client, std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)
    {
        client_ = client;
        doc_ = doc;
        enves_ = enves;
    }
    ~OnMessageTask() = default;

    /**
    * @brief 运行lua任务
    * @param tif 线程信息
    * @return 0成功,-1失败,返回失败lua线程会结束
    */
    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (client_) {
            int msgid = 0;
            if (data_) {
                msgid = data_->msgid();
            } else if (doc_) {
                msgid = (*doc_)["msgid"].GetInt();
            }
            auto cb = [&]() {
                shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
                if (data_) {
                    state["onMessage"].call<void>(client_.get(), data_->msgid(), data_->msgdata(), enves_.get());
                } else if (doc_) {
                    state["onMessage"].call<void>(client_.get(), doc_, enves_.get());
                }
                shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
            };

#ifdef USE_DEBUG
            std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(msgid));
            shynet::utils::elapsed("str");
            cb();
#elif
            cb();
#endif
        }
        return 0;
    }

private:
    /**
     * @brief protocbuf数据
    */
    std::shared_ptr<protocc::CommonObject> data_;
    /**
     * @brief json数据
    */
    std::shared_ptr<rapidjson::Document> doc_;
    /**
     * @brief 对端连接
    */
    std::shared_ptr<T> client_;
    /**
     * @brief 路由信息
    */
    std::shared_ptr<std::stack<FilterData::Envelope>> enves_;
};
}

#endif
