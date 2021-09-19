#pragma once
#include "shynet/Basic.h"
#include <functional>
#include <random>
#include <set>
#include <vector>

namespace shynet {
	namespace utils {
		/*
		* 杂七杂八
		*/
		class Stuff final : public Nocopy {
			Stuff() = delete;
		public:
			/*
			* 创建dump
			*/
			static void create_coredump();
			/*
			* 守护进程运行
			* 返回进程id
			*/
			static int daemon();

			/*
			* 获取当前进程运行目录和程序名称
			*/
			static int executable_path(char* processdir, char* processname, size_t len);

			/*
			* 写进程id
			* pidfile 默认工作目录下,程序名.pid
			*/
			static void writepid(const char* pidfile = nullptr);
			static void writepid(const std::string& pidfile);

			/*
			* 随机字符串
			*/
			static void random(void* buf, size_t len);
			/*
			* 随机[min,max]范围
			*/
			template<class T>
			static T random(T min, T max) {
				static std::random_device rd; // Non-deterministic seed source
				static std::default_random_engine rng{ rd() }; // Create random number generator
				std::uniform_int_distribution<T> num_random(min, max);
				return num_random(rng);
			}

			/*
			* uint64_t字节序转换
			*/
			static uint64_t hl64ton(uint64_t host);
			static uint64_t ntohl64(uint64_t host);

			/**
			* 将以字节为单位的数字转换为带大小单位的可读性字符串
			*
			* @param n bytes
			*
			* @return
			*   e.g.
			*   readable_bytes(768) = "768.0B"
			*   readable_bytes(10000) = "9.8K"
			*   readable_bytes(100001221) = "95.4M"
			*
			*/
			static std::string readable_bytes(uint64_t n);

			// 将字节流逐个序列化为16进制FF格式，空格分开，<num_per_line>换行，可用于debug显示
			static std::string bytes_to_hex(const uint8_t* buf, std::size_t len, std::size_t num_per_line = 8, bool with_ascii = true);
		
			/**
		    * 获取域名对应的ip
		    * @param domain 域名
		    * @return ip
		    *   成功例子：
		    *     "www.baidu.com"
		    *     "localhost"
		    *     "58.96.168.38"
		    *   失败例子：
		    *     "not exist"
		    *     "http://www.baidu.com"
		    */
			static std::string gethostbyname(const char* domain);

			// 获取当前时间点，单位毫秒，一般用于计算两个时间点间的间隔用
			static uint64_t tick_msec();
			static uint64_t unix_timestamp_msec();

			//成功返回进程当前线程数，失败返回-1
			static int32_t num_of_threads();

			// 进程启动时的unix时间戳，单位秒
			static int64_t boot_timestamp();

			// 进程至今运行时间，单位秒
			static int64_t up_duration_seconds();

			struct mem_info {
				int32_t virt_kbytes; // 进程当前虚拟内存大小，对应top中的VIRT列，单位KBytes
				int32_t res_kbytes;  // 进程当前常驻内存大小，对应top中的RES列，单位KBytes

				mem_info() : virt_kbytes(0), res_kbytes(0) {}
			};
			static bool obtain_mem_info(mem_info* mi);

			// 获取所有网卡名称
			static bool net_interfaces(std::set<std::string>* ifs);

			/**
		    * 获取当前时刻网卡收发数据量，可在两个时间点调用相减得到带宽
		    *
		    * @param interface 网卡名称 e.g. eth0
		    * @param in        传出参数，当前共收了多少字节
		    * @param out       传出参数，当前共发了多少字节
		    *
		    * @return 成功返回true，失败返回false
		    *
		    */
			static bool net_interface_bytes(const std::string& netname, uint64_t* in, uint64_t* out);
		};
	}
}
