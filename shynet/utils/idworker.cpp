#include "shynet/utils/idworker.h"
#include <chrono>

namespace shynet {
namespace utils {
    SnowflakeIdWorker::SnowflakeIdWorker(UInt worker_id = 0, UInt datacenter_id = 0)
        : worker_id_(worker_id)
        , datacenter_id_(datacenter_id)
        , sequence_(0)
        , lastTimestamp_(0)
    {
    }

    SnowflakeIdWorker::~SnowflakeIdWorker()
    {
    }

    void SnowflakeIdWorker::set_worker_id(UInt worker_id)
    {
        this->worker_id_ = worker_id;
    }
    void SnowflakeIdWorker::set_data_center_id(UInt datacenter_id)
    {
        this->datacenter_id_ = datacenter_id;
    }

    /**
		* 获得下一个ID (该方法是线程安全的)
		*
		* @return SnowflakeId
		*/

    UInt64 SnowflakeIdWorker::nextid()
    {
        std::unique_lock<std::mutex> lock { mutex_ };
        AtomicUInt64 timestamp { 0 };
        timestamp = timegen();

        // 如果当前时间小于上一次ID生成的时间戳，说明系统时钟回退过这个时候应当抛出异常
        if (timestamp < lastTimestamp_) {
            std::ostringstream s;
            s << "clock moved backwards.  Refusing to generate id for " << lastTimestamp_ - timestamp << " milliseconds";
            throw std::exception(std::runtime_error(s.str()));
        }

        if (lastTimestamp_ == timestamp) {
            // 如果是同一时间生成的，则进行毫秒内序列
            sequence_ = (sequence_ + 1) & sequence_mask_;
            if (0 == sequence_) {
                // 毫秒内序列溢出, 阻塞到下一个毫秒,获得新的时间戳
                timestamp = til_next_millis(lastTimestamp_);
            }
        } else {
            sequence_ = 0;
        }
        lastTimestamp_ = timestamp.load();

        // 移位并通过或运算拼到一起组成64位的ID
        return ((timestamp - twepoch_) << timestamp_left_shift_)
            | (datacenter_id_ << datacenter_id_shift_)
            | (worker_id_ << worker_id_shift_)
            | sequence_;
    }

    /**
		* 返回以毫秒为单位的当前时间
		*
		* @return 当前时间(毫秒)
		*/
    UInt64 SnowflakeIdWorker::timegen() const
    {
        auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
        return t.time_since_epoch().count();
    }

    /**
		* 阻塞到下一个毫秒，直到获得新的时间戳
		*
		* @param lastTimestamp 上次生成ID的时间截
		* @return 当前时间戳
		*/
    UInt64 SnowflakeIdWorker::til_next_millis(UInt64 lastTimestamp) const
    {
        UInt64 timestamp = timegen();
        while (timestamp <= lastTimestamp) {
            timestamp = timegen();
        }
        return timestamp;
    }
}
}
