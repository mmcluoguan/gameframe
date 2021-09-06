#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "shynet/Singleton.h"
#include "world/HttpClient.h"

namespace world {
	/// <summary>
	/// http后台连接管理器
	/// </summary>
	class HttpClientMgr final : public shynet::Nocopy {
		friend class shynet::Singleton<HttpClientMgr>;
		HttpClientMgr();
	public:
		~HttpClientMgr();

		/*
		* 添加,删除,查找连接,k为连接fd
		*/
		void add(int k, std::shared_ptr<HttpClient> v);
		bool remove(int k);
		std::shared_ptr<HttpClient> find(int k);

		/// <summary>
		/// 所有连接列表,int为连接fd
		/// </summary>
		/// <returns></returns>
		std::unordered_map<int, std::shared_ptr<HttpClient>> clis() const;

		/*
		* 获取设置http后台服务器监听地址
		*/
		const net::IPAddress& listen_addr() const;
		void listen_addr(const net::IPAddress& addr);
	private:
		net::IPAddress listen_addr_;
		mutable std::mutex clis_mutex_;
		std::unordered_map<int, std::shared_ptr<HttpClient>> clis_;
	};
}
