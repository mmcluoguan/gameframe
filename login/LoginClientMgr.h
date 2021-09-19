#pragma once
#include "login/LoginClient.h"
#include "shynet/utils/Singleton.h"
#include <unordered_map>

namespace login
{
	/// <summary>
	/// 登录服连接管理器
	/// </summary>
	class LoginClientMgr final : public shynet::Nocopy
	{
		friend class shynet::utils::Singleton<LoginClientMgr>;
		LoginClientMgr();
	public:
		~LoginClientMgr();

		/*
        * 添加,删除,查找连接,k为连接fd
        */
		void add(int k, std::shared_ptr<LoginClient> v);
		bool remove(int k);
		std::shared_ptr<LoginClient> find(int k);
		/*
		* 通过服务id查找连接
		*/
		std::shared_ptr<LoginClient> find_from_sid(const std::string& sid) const;

		/// <summary>
        /// 所有连接列表,int为连接fd
        /// </summary>
        /// <returns></returns>
		std::unordered_map<int, std::shared_ptr<LoginClient>> clis() const;

		/*
        * 获取设置登录服务器监听地址
        */
		const net::IPAddress& listen_addr() const;
		void set_listen_addr(const net::IPAddress& addr);
	private:
		net::IPAddress listen_addr_;
		mutable std::mutex clis_mutex_;
		std::unordered_map<int, std::shared_ptr<LoginClient>> clis_;
	};
}
