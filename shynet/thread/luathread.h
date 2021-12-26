#ifndef SHYNET_THREAD_LUATHREAD_H
#define SHYNET_THREAD_LUATHREAD_H

#include "3rd/kaguya/kaguya.hpp"
#include "shynet/lua/luatask.h"
#include "shynet/thread/thread.h"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    /**
     * @brief lua�߳�
    */
    class LuaThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е�����
        */
        explicit LuaThread(size_t index);
        ~LuaThread() = default;

        /**
         * @brief �߳����лص�
         * @return 0�ɹ� -1ʧ��
        */
        int run() override;
        /**
         * @brief ��ȫ��ֹ�߳�
         * @return 0�ɹ� -1ʧ��
        */
        int stop() override;

        /**
         * @brief ���lua����
         * @param tk lua����
        */
        void addTask(std::shared_ptr<luatask::LuaTask> tk);

        /**
         * @brief ��ȡlua״̬
         * @return lua״̬
        */
        kaguya::State* luaState() const { return luaState_; }

    private:
        /**
         * @brief ������
        */
        std::mutex tasks_mutex_;
        /**
         * @brief ��������
        */
        std::condition_variable tasks_condvar_;
        /**
         * @brief lua�������
        */
        std::queue<std::shared_ptr<luatask::LuaTask>> tasks_;
        /**
         * @brief �߳̽�����ʶ
        */
        bool stop_ = false;
        /**
         * @brief lua״̬
        */
        kaguya::State* luaState_ = nullptr;
    };
}
}

#endif
