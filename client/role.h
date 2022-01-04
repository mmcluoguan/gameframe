#ifndef CLIENT_ROLE_H
#define CLIENT_ROLE_H

#include "frmpub/filterdata.h"
#include "shynet/basic.h"
#include "shynet/utils/singleton.h"
#include <list>

using namespace shynet;
using namespace frmpub;

namespace client {

class GateConnector;
class Role {

public:
    struct Goods {
        int64_t id;
        int32_t cfgid;
        int32_t num;
    };

    struct Email {
        struct Annex {
            int gold;
            int diamond;
            std::list<Goods> goodslist;
        };

        int64_t id;
        bool is_read;
        bool is_receive;
        std::string title;
        std::string info;
        int type;
        int time;

        //附件
        std::shared_ptr<Annex> annex;
    };

    /**
     * @brief 构造
    */
    Role();
    ~Role() = default;

    /**
     * @brief 获取角色id
     * @return 角色id
    */
    int64_t id() const { return id_; }
    /**
     * @brief 设置角色id
     * @param v 角色id
    */
    void set_id(int64_t v) { id_ = v; }
    /**
     * @brief 获取账号
     * @return 账号
    */
    std::string accountid() const { return accountid_; }
    /**
     * @brief 设置账号
     * @param v 账号
    */
    void set_accountid(std::string v) { accountid_ = v; }

    /**
     * @brief 获取等级
     * @return 等级
    */
    int32_t level() const { return level_; }
    /**
     * @brief 设置等级
     * @param v 等级
    */
    void set_level(int32_t v) { level_ = v; }

    /**
     * @brief 获取金币
     * @return 金币
    */
    int32_t gold() const { return gold_; }
    /**
     * @brief 设置金币
     * @param v 金币
    */
    void set_gold(int32_t v) { gold_ = v; }

    /**
     * @brief 获取钻石
     * @return 钻石
    */
    int32_t diamond() const { return diamond_; }

    /**
     * @brief 设置钻石
     * @param v 钻石
    */
    void set_diamond(int32_t v) { diamond_ = v; }
    /**
     * @brief 获取剩余抽奖次数
     * @return 剩余抽奖次数
    */
    int32_t lottery() const { return lottery_; }

    /**
     * @brief 设置剩余抽奖次数
     * @param v 剩余抽奖次数
    */
    void set_lottery(int32_t v) { lottery_ = v; }

    /**
     * @brief 设置对应连接
     * @param v 对应连接
    */
    void set_gate(std::weak_ptr<GateConnector> v) { gate_ = v; }
    /**
     * @brief 获取背包hash表
     * @return 背包hash表
    */
    std::unordered_map<int64_t, Goods>& goodsmap() { return goodsmap_; }

    int send_proto(int msgid, const google::protobuf::Message* data = nullptr,
        std::stack<FilterData::Envelope>* enves = nullptr, const std::string* extend = nullptr) const;

    int send_errcode(protocc::errnum code, const std::string& desc,
        std::stack<FilterData::Envelope>* enves = nullptr);

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<frmpub::FilterData::Envelope>> enves);

    /*
		* 加载角色数据结果
		*/
    int loadrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 加载角色物品数据结果
		*/
    int loadgoods_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
	* 更新角色物品数据
	*/
    int goodsupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 区服服务器广播信息
		*/
    int notice_info_clent_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 广播信息列表
		*/
    int notice_info_list_clent_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 新邮件通知
		*/
    int email_new_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 设置角色等级
		*/
    int setlevel_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 加载邮件列表
		*/
    int loademails_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 查看邮件信息
		*/
    int lookemail_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 领取邮件附件
		*/
    int getannex_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 基础数据更新
		*/
    int baseupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /**
     * @brief 角色id
    */
    int64_t id_;
    /**
     * @brief 账号id
    */
    std::string accountid_;
    /**
     * @brief 对应连接
    */
    std::weak_ptr<GateConnector> gate_;
    /**
     * @brief 等级
    */
    int32_t level_;
    /**
     * @brief 金币
    */
    int32_t gold_;
    /**
     * @brief 钻石
    */
    int32_t diamond_;
    /**
     * @brief 剩余抽奖次数
    */
    int32_t lottery_;
    /**
     * @brief 背包hash表
    */
    std::unordered_map<int64_t, Goods> goodsmap_;
    /**
     * @brief 邮件hash表
    */
    std::unordered_map<int64_t, Email> emailsmap_;

    using ProtoMsgBind = std::function<int(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)>;
    /**
     * @brief 消息处理函数hash表
    */
    std::unordered_map<int, ProtoMsgBind> pmb_;
};
}

#endif
