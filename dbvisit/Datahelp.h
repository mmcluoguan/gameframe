#ifndef DBVISIT_DATAHELP_H
#define DBVISIT_DATAHELP_H

#include "shynet/utils/Singleton.h"
#include <list>
#include <unordered_map>

namespace dbvisit {
	using moredata = std::list<std::unordered_map<std::string, std::string>>;
	using moredataptr = std::shared_ptr<moredata>;

	/// <summary>
	/// 数据简易操作
	/// </summary>
	class Datahelp :public shynet::Nocopy {
		friend class shynet::utils::Singleton<Datahelp>;
		Datahelp();
	public:
		/// <summary>
		/// 错误码
		/// </summary>
		enum class ErrorCode {
			OK,
			NOT_DATA, //没有查询到数据
		};

		/// <summary>
		/// 从db获取1条hash数据，
		/// 筛选条件优先使用key,只有当key为空字符串时，才使用where
		/// </summary>
		/// <param name="tablename">表名</param>
		/// <param name="key">主键</param>
		/// <param name="out">输出字典</param>
		/// <param name="where">条件</param>
		/// <returns></returns>
		ErrorCode getdata_from_db(const std::string& tablename,
			const std::string& key, std::unordered_map<std::string, std::string>& out,
			const std::string& where = "");

		/// <summary>
		/// 从cache获取唯一1条hash数据
		/// </summary>
		/// <param name="cachekey">cachekey定义规则为tablename_key</param>
		/// <param name="out">输出字典</param>
		/// <param name="seconds">cache过期时间,默认24小时,值为0表示永不过期</param>
		/// <returns></returns>
		ErrorCode getdata_from_cache(const std::string& cachekey,
			std::unordered_map<std::string, std::string>& out,
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

		/// <summary>
		/// 优先从cache获取1条hash数据,cache没有则从db获取，
		/// 然后updatacache决定是否更新cache
		/// </summary>
		/// <param name="cachekey">cachekey定义规则为tablename_key</param>
		/// <param name="out">输出字典</param>
		/// <param name="updatacache">从db获取后是否最后更新cache</param>
		/// <param name="seconds">cache过期时间,默认24小时,值为0表示永不过期</param>
		/// <returns></returns>
		ErrorCode getdata(const std::string& cachekey,
			std::unordered_map<std::string, std::string>& out,
			bool updatacache = true,
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

		/*
		* 指定条件从db获取多条hash数据
		*/
		moredataptr getdata_more_db(const std::string& tablename,
			const std::string& condition,
			std::unordered_map<std::string, std::string>& out);

		/*
		* 指定条件从cache获取多条hash数据
		*/
		moredataptr getdata_more_cache(const std::string& condition,
			std::unordered_map<std::string, std::string>& out);

		/// <summary>
		/// 优先从cache获取多条hash数据, cache没有则从db获取，
		/// 然后updatacache决定是否更新cache
		/// </summary>
		/// <param name="condition">condition定义规则为tablename_*_roleid</param>
		/// <param name="out">输出字典</param>
		/// <param name="updatacache">从db获取后是否最后更新cache</param>
		/// <param name="seconds">cache过期时间,默认24小时,值为0表示永不过期</param>
		/// <returns>多条hash数据</returns>
		moredataptr getdata_more(const std::string& condition,
			std::unordered_map<std::string, std::string>& out,
			bool updatacache = true,
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
		/*
		* 保存hash数据到db
		*/
		void updata_db(const std::string& tablename, const std::string& key,
			const std::unordered_map<std::string, std::string>& fields);

		/*
		* 插入hash数据到db
		*/
		void insert_db(const std::string& tablename, const std::string& key,
			const std::unordered_map<std::string, std::string>& fields);

		/*
		* 从dv删除数据
		*/
		void delete_db(const std::string& tablename, const std::string& key);

		/*
		* 先插入hash数据到cache,再立刻保存到db
		* seconds cache过期时间,默认24小时,值为0表示永不过期
		*/
		void insertdata(const std::string& cachekey,
			const std::unordered_map<std::string, std::string>& fields,
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

		/*
		* 先删除cache中hash数据,再立即删除db中hash数据
		*/
		void deletedata(const std::string& cachekey);

		/*
		* 更新cache
		* seconds cache过期时间,默认24小时,值为0表示永不过期
		*/
		void updata_cache(const std::string& cachekey,
			const std::unordered_map<std::string, std::string>& fields,
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

		/// <summary>
		/// 先保持hash数据到cache,再保存到db
		/// </summary>
		/// <param name="cachekey">cachekey定义规则为tablename_key</param>
		/// <param name="fields"></param>
		/// <param name="immediately">是否立即保存db</param>
		/// <param name="timeval">延迟写db时间(s)</param>
		/// <param name="seconds">cache过期时间,默认24小时,值为0表示永不过期</param>
		/// <returns></returns>
		void updata(const std::string& cachekey,
			const std::unordered_map<std::string, std::string>& fields,
			bool immediately = false,
			const timeval val = { 10,0 },
			std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
	};
}

#endif
