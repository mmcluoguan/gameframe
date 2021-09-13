#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "shynet/utils/Singleton.h"
#include "dbvisit/DbClient.h"

namespace dbvisit {
	/// <summary>
	/// db连接管理器
	/// </summary>
	class DbClientMgr final : public shynet::Nocopy {
		friend class shynet::utils::Singleton<DbClientMgr>;
		DbClientMgr();
	public:
		~DbClientMgr();

		/*
		* 添加,删除,查找连接,k为连接fd
		*/
		void add(int k, std::shared_ptr<DbClient> v);
		bool remove(int k);
		std::shared_ptr<DbClient> find(int k);

		/// <summary>
		/// 所有连接列表,int为连接fd
		/// </summary>
		/// <returns></returns>
		std::unordered_map<int, std::shared_ptr<DbClient>> clis() const;

		/*
		* 获取设置db服务器监听地址
		*/
		const net::IPAddress& listen_addr() const;
		void set_listen_addr(const net::IPAddress& addr);

		/*
		* 通过服务器id查找
		*/
		std::shared_ptr<DbClient> find(std::string sid);
	private:
		net::IPAddress listen_addr_;
		mutable std::mutex clis_mutex_;
		std::unordered_map<int, std::shared_ptr<DbClient>> clis_;
	};
}
