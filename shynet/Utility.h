#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "shynet/Basic.h"

namespace shynet {
	/*
	* 实用工具
	*/
	class Utility final : public Nocopy {
		Utility() = delete;
	public:
		/*
		* 创建dump
		*/
		static void create_coredump();
		/*
		* 守护进程运行
		*/
		static void daemon();

		static char* trim(char* str, std::function<int(int)> cb = isspace);
		static std::string& trim(std::string& xstr, std::function<int(int)> cb = isspace);

		/// <summary>
		/// 拆分字符串
		/// </summary>
		/// <param name="target">目标</param>
		/// <param name="c">分割符</param>
		/// <param name="argv">拆分到的目标字符串数组</param>
		/// <param name="size">目标字符串数组大小</param>
		static int spilt(char* target, const char* c, char** argv, int size);
		static std::vector<std::string> spilt(const std::string& target, const char* c);

		static int get_executable_path(char* processdir, char* processname, size_t len);

		template<typename ... Args>
		static std::string str_format(const std::string& format, Args ... args) {
			auto size_buf = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1;
			std::unique_ptr<char[]> buf(new char[size_buf]);

			if (!buf)
				return std::string("");

			std::snprintf(buf.get(), size_buf, format.c_str(), std::forward<Args>(args)...);
			return std::string(buf.get(), buf.get() + size_buf - 1);
		}

		static void random(void* buf, size_t len);

		static uint64_t hl64ton(uint64_t host);
		static uint64_t ntohl64(uint64_t host);
	private:
	};
}
