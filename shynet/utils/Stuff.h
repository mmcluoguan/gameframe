#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include "shynet/Basic.h"

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
				std::default_random_engine random(time(nullptr));
				std::uniform_int_distribution<T> num_random(min, max);
				return num_random(random);
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
		private:
		};
	}
}
