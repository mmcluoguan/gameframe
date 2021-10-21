#ifndef FRMPUB_ONACCEPTTASK_H
#define FRMPUB_ONACCEPTTASK_H

#include "frmpub/LuaRemoteDebug.h"
#include "shynet/task/Task.h"
#include "shynet/thread/LuaThread.h"
#include "shynet/thread/Thread.h"

namespace frmpub {
/*
	* 处理新连接
	*/
template <class T>
class OnAcceptTask : public shynet::task::Task {
public:
    OnAcceptTask(std::shared_ptr<T> client)
    {
        client_ = client;
    }
    ~OnAcceptTask()
    {
    }

    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (client_) {
            shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
            state["onAccept"].call<void>(client_.get());
            shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        }
        return 0;
    }

private:
    std::shared_ptr<T> client_;
};

/*
	* 处理连接成功
	*/
template <class T>
class OnConnectorTask : public shynet::task::Task {
public:
    OnConnectorTask(std::shared_ptr<T> conncetor)
    {
        conncetor_ = conncetor;
    }
    ~OnConnectorTask()
    {
    }

    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (conncetor_) {
            shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
            state["onConnect"].call<void>(conncetor_.get());
            shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        }
        return 0;
    }

private:
    std::shared_ptr<T> conncetor_;
};

/*
	* 处理连接关闭
	*/
class OnCloseTask : public shynet::task::Task {
public:
    OnCloseTask(int fd)
    {
        fd_ = fd;
    }
    ~OnCloseTask()
    {
    }

    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
        state["onClose"].call<void>(fd_);
        shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        return 0;
    }

private:
    int fd_;
};

/*
	* 处理连接数据
	*/
template <class T>
class OnMessageTask : public shynet::task::Task {
public:
    OnMessageTask(std::shared_ptr<T> client, std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)
    {
        client_ = client;
        data_ = data;
        enves_ = enves;
    }
    OnMessageTask(std::shared_ptr<T> client, std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)
    {
        client_ = client;
        doc_ = doc;
        enves_ = enves;
    }
    ~OnMessageTask()
    {
    }

    int run(thread::Thread* tif) override
    {
        thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
        kaguya::State& state = *(lua->luaState());
        if (client_) {
            shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
            if (data_) {
                state["onMessage"].call<void>(client_.get(), data_->msgid(), data_->msgdata(), enves_.get());
            } else if (doc_) {
                state["onMessage"].call<void>(client_.get(), doc_, enves_.get());
            }
            shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        }
        return 0;
    }

private:
    std::shared_ptr<protocc::CommonObject> data_;
    std::shared_ptr<rapidjson::Document> doc_;
    std::shared_ptr<T> client_;
    std::shared_ptr<std::stack<FilterData::Envelope>> enves_;
};
}

#endif
