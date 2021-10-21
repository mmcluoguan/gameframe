#ifndef SHYNET_UTILS_IDWORKER_H
#define SHYNET_UTILS_IDWORKER_H

#include "shynet/utils/Singleton.h"
#include <atomic>

namespace shynet {
namespace utils {

    using UInt = uint32_t;
    using UInt64 = uint64_t;

    using AtomicUInt = std::atomic<UInt>;
    using AtomicUInt64 = std::atomic<UInt64>;

    /**
		 * 64bit id: 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
		 *           ||                                                           ||     ||     |  |              |
		 *           |└---------------------------时间戳--------------------------┘└中心-┘└机器-┘  └----序列号----┘
		 *           |
		 *         不用
		 * SnowFlake的优点: 整体上按照时间自增排序, 并且整个分布式系统内不会产生ID碰撞(由数据中心ID和机器ID作区分), 并且效率较高, 经测试, SnowFlake每秒能够产生26万ID左右.
		 */
    class SnowflakeIdWorker final : public shynet::Nocopy {

        // 实现单例
        friend class Singleton<SnowflakeIdWorker>;

    public:
        static constexpr const char* kClassname = "SnowflakeIdWorker";
        ~SnowflakeIdWorker();

        void set_worker_id(UInt worker_id);

        void set_data_center_id(UInt datacenter_id);

        UInt64 getid()
        {
            return nextid();
        }

        /**
			 * 获得下一个ID (该方法是线程安全的)
			 *
			 * @return SnowflakeId
			 */
        UInt64 nextid();

    private:
        SnowflakeIdWorker(UInt worker_id, UInt datacenter_id);

        /**
			 * 返回以毫秒为单位的当前时间
			 *
			 * @return 当前时间(毫秒)
			 */
        UInt64 timegen() const;

        /**
			 * 阻塞到下一个毫秒，直到获得新的时间戳
			 *
			 * @param lastTimestamp 上次生成ID的时间截
			 * @return 当前时间戳
			 */
        UInt64 til_next_millis(UInt64 lastTimestamp) const;

    private:
        std::mutex mutex_;

        /**
			 * 开始时间截 (2018-01-01 00:00:00.000)
			 */
        const UInt64 twepoch_ = 1514736000000;

        /**
			 * 机器id所占的位数
			 */
        const UInt worker_id_bits_ = 5;

        /**
			 * 数据中心id所占的位数
			 */
        const UInt data_center_id_bits_ = 5;

        /**
			 * 序列所占的位数
			 */
        const UInt sequence_bits_ = 12;

        /**
			 * 机器ID向左移12位
			 */
        const UInt worker_id_shift_ = sequence_bits_;

        /**
			 * 数据标识id向左移17位
			 */
        const UInt datacenter_id_shift_ = worker_id_shift_ + worker_id_bits_;

        /**
			 * 时间截向左移22位
			 */
        const UInt timestamp_left_shift_ = datacenter_id_shift_ + data_center_id_bits_;

        /**
			 * 支持的最大机器id，结果是31
			 */
        const UInt max_worker_id_ = -1 ^ (-1 << worker_id_bits_);

        /**
			 * 支持的最大数据中心id，结果是31
			 */
        const UInt max_datacenter_id_ = -1 ^ (-1 << data_center_id_bits_);

        /**
			 * 生成序列的掩码，这里为4095
			 */
        const UInt sequence_mask_ = -1 ^ (-1 << sequence_bits_);

        /**
			 * 工作机器id(0~31)
			 */
        UInt worker_id_;

        /**
			 * 数据中心id(0~31)
			 */
        UInt datacenter_id_;

        /**
			 * 毫秒内序列(0~4095)
			 */
        AtomicUInt sequence_ { 0 };

        /**
			 * 上次生成ID的时间截
			 */
        AtomicUInt64 lastTimestamp_ { 0 };
    };

    /*
		* id生成器
		*/
    using IdWorker = SnowflakeIdWorker;
}
}

#endif
