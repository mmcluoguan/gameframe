#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "shynet/Singleton.h"
#include "gate/GateClient.h"

namespace gate {
	/// <summary>
	/// 网关服连接管理器
	/// </summary>
	class GateClientMgr final : public shynet::Nocopy {
		friend class shynet::Singleton<GateClientMgr>;
		GateClientMgr();
	public:
		~GateClientMgr();

		/*
		* 添加,删除,查找连接,k为连接fd
		*/
		void add(int k, std::shared_ptr<GateClient> v);
		bool remove(int k);
		std::shared_ptr<GateClient> find(int k);
		/*
		* 通过账号查找
		*/
		std::shared_ptr<GateClient> find(const std::string& accountid);
		/*
		* 通过名称和密码查找
		*/
		std::shared_ptr<GateClient> find(const std::string& name, const std::string& pwd);

		/// <summary>
		/// 所有连接列表,int为连接fd
		/// </summary>
		/// <returns></returns>
		std::unordered_map<int, std::shared_ptr<GateClient>> clis() const;

		/*
		* 获取设置登录服务器监听地址
		*/
		const net::IPAddress& listen_addr() const;
		void listen_addr(const net::IPAddress& addr);

	private:
		net::IPAddress listen_addr_;
		mutable std::mutex clis_mutex_;
		std::unordered_map<int, std::shared_ptr<GateClient>> clis_;
	};
}
