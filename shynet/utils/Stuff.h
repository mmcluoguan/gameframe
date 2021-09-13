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
			static int get_executable_path(char* processdir, char* processname, size_t len);

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
		private:
		};
	}
}
