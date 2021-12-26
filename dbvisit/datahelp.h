#ifndef DBVISIT_DATAHELP_H
#define DBVISIT_DATAHELP_H

#include "shynet/utils/singleton.h"
#include <list>
#include <unordered_map>

namespace dbvisit {
using moredata = std::list<std::unordered_map<std::string, std::string>>;
using moredataptr = std::shared_ptr<moredata>;

/**
 * @brief 数据简易操作器
*/
class Datahelp : public shynet::Nocopy {
    friend class shynet::utils::Singleton<Datahelp>;
    Datahelp() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "Datahelp";

    /**
     * @brief 操作类型
    */
    enum class OperType {
        /**
         * @brief 先操作cache,然后操作db
        */
        ALL = 0,
        /**
         * @brief 只操作cache
        */
        CACHE,
        /**
         * @brief 只操作db
        */
        DB,
    };
    /**
     * @brief 操作结果码
    */
    enum class ErrorCode {
        /**
         * @brief 成功
        */
        OK,
        /**
         * @brief 没有查询到数据
        */
        NOT_DATA,
    };

    /**
     * @brief 从db获取1条hash数据
     筛选条件优先使用key,只有当key为空字符串时，才使用where
     * @param tablename 表名
     * @param key 主键
     * @param out 输出字段hash表
     * @param where 条件
     * @return 操作结果码
    */
    ErrorCode getdata_from_db(const std::string& tablename,
        const std::string& key, std::unordered_map<std::string, std::string>& out,
        const std::string& where = "");

    /**
     * @brief 从cache获取唯一1条hash数据
     * @param cachekey cachekey定义规则为tablename_key
     * @param out 输出字段hash表
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
     * @return 操作结果码
    */
    ErrorCode getdata_from_cache(const std::string& cachekey,
        std::unordered_map<std::string, std::string>& out,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
    /**
     * @brief 优先从cache获取1条hash数据,cache没有则从db获取，
    然后updatacache决定是否更新cache     
    * @param cachekey cachekey定义规则为tablename_key
     * @param out 输出字段hash表
     * @param opertype ALL先查找cache,然后查找db,CACHE只查找cache,NOT_DATA只查找db
     * @param updatacache 从db获取后是否最后更新cache
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
     * @return 操作结果码
    */
    ErrorCode getdata(const std::string& cachekey,
        std::unordered_map<std::string, std::string>& out,
        OperType opertype = OperType::ALL,
        bool updatacache = true,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

    /**
     * @brief 指定条件从db获取多条hash数据
     * @param tablename 表名
     * @param condition 条件
     * @param out 输出字段hash表
     * @param sort 排序
     * @param limit 限制结果条数
     * @return 多条hash数据
    */
    moredataptr getdata_more_db(const std::string& tablename,
        const std::string& condition,
        std::unordered_map<std::string, std::string>& out,
        std::string sort = "",
        int32_t limit = 0);

    /**
     * @brief 指定条件从cache获取多条hash数据
     * @param condition 条件
     * @param out 输出字段hash表
     * @param sort 排序字符串,sort=""表示不排序,否则如 age asc 或者 age desc
     * @param limit 最多取多少条数据,0表示获取满足条件的所有数据
     * @return 多条hash数据
    */
    moredataptr getdata_more_cache(const std::string& condition,
        std::unordered_map<std::string, std::string>& out,
        std::string sort = "",
        int32_t limit = 0);
    /**
     * @brief 优先从cache获取多条hash数据, cache没有则从db获取，
    然后updatacache决定是否更新cache
     * @param condition condition定义规则为tablename_*_roleid
     * @param out 输出字段hash表
     * @param sort 排序字符串,sort=""表示不排序,否则如 age asc 或者 age desc
     * @param limit 最多取多少条数据,0表示获取满足条件的所有数据
     * @param opertype ALL先查找cache,然后查找db,CACHE只查找cache,NOT_DATA只查找db
     * @param updatacache 从db获取后是否最后更新cache
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
     * @return 多条hash数据
    */
    moredataptr getdata_more(const std::string& condition,
        std::unordered_map<std::string, std::string>& out,
        std::string sort = "",
        int32_t limit = 0,
        OperType opertype = OperType::ALL,
        bool updatacache = true,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

    /**
     * @brief 插入hash数据到db
     * @param tablename 表名
     * @param key 主键
     * @param fields hash数据
    */
    void insert_db(const std::string& tablename, const std::string& key,
        const std::unordered_map<std::string, std::string>& fields);

    /**
     * @brief 插入hash数据到cache
     * @param cachekey cachekey定义规则为tablename_key
     * @param fields hash数据
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
    */
    void insert_cache(const std::string& cachekey,
        const std::unordered_map<std::string, std::string>& fields,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
    /**
     * @brief 先插入hash数据到cache,再立刻保存到db
     * @param cachekey cachekey定义规则为tablename_key
     * @param fields hash数据
     * @param opertype ALL先查找cache,然后查找db,CACHE只查找cache,NOT_DATA只查找db
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
    */
    void insertdata(const std::string& cachekey,
        const std::unordered_map<std::string, std::string>& fields,
        OperType opertype = OperType::ALL,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));

    /**
     * @brief 从db删除数据
     * @param tablename 表名
     * @param key 主键
    */
    void delete_db(const std::string& tablename, const std::string& key);

    /**
     * @brief 从cache删除数据
     * @param cachekey cachekey定义规则为tablename_key
    */
    void delete_cache(const std::string& cachekey);

    /**
     * @brief 先删除cache中hash数据,再立即删除db中hash数据
     * @param cachekey cachekey定义规则为tablename_key
     * @param opertype ALL先查找cache,然后查找db,CACHE只查找cache,NOT_DATA只查找db
    */
    void deletedata(const std::string& cachekey, OperType opertype = OperType::ALL);

    /**
     * @brief 保存hash数据到db
     * @param tablename 表名
     * @param key 主键
     * @param fields hash数据
    */
    void updata_db(const std::string& tablename, const std::string& key,
        const std::unordered_map<std::string, std::string>& fields);

    /**
     * @brief 更新cache
     * @param cachekey cachekey定义规则为tablename_key
     * @param fields hash数据
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期 
    */
    void updata_cache(const std::string& cachekey,
        const std::unordered_map<std::string, std::string>& fields,
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
    /**
     * @brief 先保存hash数据到cache,再保存到db
     * @param cachekey cachekey定义规则为tablename_key
     * @param fields hash数据
     * @param opertype ALL先查找cache,然后查找db,CACHE只查找cache,NOT_DATA只查找db
     * @param immediately 是否立即保存db
     * @param val 延迟写db时间(s)
     * @param seconds cache过期时间,默认24小时,值为0表示永不过期
    */
    void updata(const std::string& cachekey,
        const std::unordered_map<std::string, std::string>& fields,
        OperType opertype = OperType::ALL,
        bool immediately = false,
        const timeval val = { 10, 0 },
        std::chrono::seconds seconds = std::chrono::seconds(24 * 60 * 60));
};
}

#endif
