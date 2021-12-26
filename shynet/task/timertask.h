#ifndef SHYNET_TASK_TIMERTASK_H
#define SHYNET_TASK_TIMERTASK_H

namespace shynet {
namespace task {
    /**
     * @brief ��ʱ����ʱ��������
    */
    class TimerTask {
    public:
        /**
         * @brief ����
         * @param timerid ��ʱ��id 
        */
        explicit TimerTask(int timerid);
        ~TimerTask() = default;

        /**
         * @brief ������
        */
        void operator()();

    private:
        /**
         * @brief ��ʱ��id 
        */
        int timerid_ = 0;
    };
}
}

#endif
