#include "frmpub/filterdata.h"
#include "3rd/fmt/format.h"
#include "3rd/rapidjson/stringbuffer.h"
#include "3rd/rapidjson/writer.h"
#include "frmpub/responsetimer.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/singleton.h"
#include <memory>

namespace frmpub {
FilterData::FilterData(ProtoData pd)
{
    pd_ = pd;
}
int FilterData::message_handle(char* original_data, size_t datalen)
{
    int ret = 0;
    if (pd_ == ProtoData::NATIVE) {
        ret = native_handle(original_data, datalen);
    } else if (pd_ == ProtoData::PROTOBUF) {
        ret = protobuf_handle(original_data, datalen);
    } else if (pd_ == ProtoData::JSON) {
        ret = json_handle(original_data, datalen);
    } else {
        LOG_WARN << "未知封包类型";
        return -1;
    }
    return ret;
}

int FilterData::protobuf_handle(const char* original_data, size_t datalen)
{
    std::shared_ptr<protocc::CommonObject> obj = std::make_shared<protocc::CommonObject>();
    if (obj->ParseFromArray(original_data, (int)datalen) == true) {
        std::shared_ptr<std::stack<Envelope>> enves = std::make_shared<std::stack<Envelope>>();
        for (int i = obj->rs_size() - 1; i >= 0; i--) {
            Envelope enve;
            enve.fd = obj->rs(i).fd();
            memcpy(&enve.addr, obj->rs(i).addr().c_str(), sizeof(enve.addr));
            enves->push(enve);
        }
        return input_handle(obj, enves);
    } else {
        LOG_WARN << "protocc::CommonObject 反序列化失败";
        return -1;
    }
}

int FilterData::json_handle(const char* original_data, size_t datalen)
{
    std::shared_ptr<rapidjson::Document> doc = std::make_shared<rapidjson::Document>();
    rapidjson::Value& obj = doc->Parse(original_data, datalen);
    if (obj.IsObject()) {
        std::shared_ptr<std::stack<Envelope>> enves = std::make_shared<std::stack<Envelope>>();
        auto iter = doc->FindMember("rs");
        if (iter != doc->MemberEnd() && iter->value.IsArray()) {
            for (rapidjson::SizeType i = iter->value.Size() - 1; i >= 0; i--) {
                rapidjson::Value& item = iter->value[i];
                auto fd_iter = item.FindMember("fd");
                auto ip_iter = item.FindMember("ip");
                auto port_iter = item.FindMember("port");
                if (fd_iter != item.MemberEnd() && fd_iter->value.IsUint() && ip_iter != item.MemberEnd() && ip_iter->value.IsString() && port_iter != item.MemberEnd() && port_iter->value.IsUint()) {
                    Envelope enve;
                    enve.fd = fd_iter->value.GetUint();
                    net::IPAddress ipadrr(
                        ip_iter->value.GetString(),
                        static_cast<unsigned short>(port_iter->value.GetUint()));
                    enve.addr = *ipadrr.sockaddr();
                    enves->push(enve);
                } else {
                    LOG_WARN << "json 字段没有fd或ip或port,或者字段类型错误";
                    return -1;
                }
            }
        }
        iter = obj.FindMember("msgid");
        if (iter != obj.MemberEnd()) {
            return input_handle(doc, enves);
        } else {
            LOG_WARN << "json 字段没有msgid错误";
            return -1;
        }
    } else {
        LOG_WARN << "json:[" << std::string(original_data, datalen) << "] 格式错误";
        return -1;
    }
}

int FilterData::native_handle(const char* original_data, size_t datalen)
{
    std::shared_ptr<std::stack<Envelope>> enves = std::make_shared<std::stack<Envelope>>();
    return input_handle(original_data, datalen, enves);
}

int FilterData::input_handle(const std::shared_ptr<protocc::CommonObject> obj,
    std::shared_ptr<std::stack<Envelope>> enves)
{
#ifdef USE_DEBUG
    {
        std::lock_guard<std::mutex> lock(rtbmut_);
        auto it = response_timer_bind_.find(obj->msgid());
        if (it != response_timer_bind_.end()) {
            //清除服务器回应消息超时计时器
            if (it->second.empty() == false) {
                int timerid = *it->second.begin();
                it->second.pop_front();
                auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
                timermgr.remove(timerid);
            }
            if (it->second.empty()) {
                response_timer_bind_.erase(obj->msgid());
            }
        }
    }
#endif
    preinput_handle(obj, enves);
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        }
        return default_handle(obj, enves);
    };

#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#else
    return cb();
#endif
};

int FilterData::input_handle(const std::shared_ptr<rapidjson::Document> obj,
    std::shared_ptr<std::stack<Envelope>> enves)
{
    int msgid = (*obj)["msgid"].GetInt();
#ifdef USE_DEBUG
    {
        std::lock_guard<std::mutex> lock(rtbmut_);
        auto it = response_timer_bind_.find(msgid);
        if (it != response_timer_bind_.end()) {
            //清除服务器回应消息超时计时器
            if (it->second.empty() == false) {
                int timerid = *it->second.begin();
                it->second.pop_front();
                auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
                timermgr.remove(timerid);
            }
            if (it->second.empty()) {
                response_timer_bind_.erase(msgid);
            }
        }
    }
#endif
    preinput_handle(obj, enves);
    auto cb = [&]() {
        auto it = jmb_.find(msgid);
        if (it != jmb_.end()) {
            return it->second(obj, enves);
        }
        return default_handle(obj, enves);
    };

#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(msgid));
    shynet::utils::elapsed(str.c_str());
    return cb();
#else
    return cb();
#endif
}

void FilterData::default_response_timeout(int msgid_c, int msgid_s)
{
#ifdef USE_DEBUG
    {
        std::lock_guard<std::mutex> lock(rtbmut_);
        auto it = response_timer_bind_.find(msgid_s);
        if (it != response_timer_bind_.end()) {
            if (it->second.empty() == false) {
                it->second.pop_back();
            }
            if (it->second.empty()) {
                response_timer_bind_.erase(msgid_s);
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(pmbmut_);
        pmb_.erase(msgid_s);
    }
    {
        std::lock_guard<std::mutex> lock(jmbmut_);
        jmb_.erase(msgid_s);
    }
    LOG_DEBUG << "客户端请求消息id:" << frmpub::Basic::msgname(msgid_c)
              << " 期望服务器回应消息id:" << frmpub::Basic::msgname(msgid_s)
              << " 超时";
#endif
}

int FilterData::send_proto(protocc::CommonObject* data,
    std::stack<Envelope>* enves) const
{
    data->clear_rs();
    std::stack<Envelope> new_enves;
    if (enves != nullptr)
        new_enves = *enves;
    while (new_enves.empty() == false) {
        Envelope& enve = new_enves.top();
        new_enves.pop();
        auto rs = data->add_rs();
        rs->set_fd(enve.fd);
        rs->set_addr(&enve.addr, sizeof(enve.addr));
    }
    return filter_->send(data->SerializePartialAsString());
}
int FilterData::send_proto(int msgid, const google::protobuf::Message* data,
    std::stack<Envelope>* enves,
    const std::string* extend) const
{
    protocc::CommonObject obj;
    if (extend != nullptr) {
        obj.set_extend(*extend);
    }
    obj.set_msgid(msgid);
    if (data != nullptr) {
        obj.set_msgdata(data->SerializePartialAsString());
    }
    return send_proto(&obj, enves);
}
int FilterData::send_proto(int msgid, const std::string data,
    std::stack<Envelope>* enves,
    const std::string* extend) const
{
    protocc::CommonObject obj;
    if (extend != nullptr) {
        obj.set_extend(*extend);
    }
    obj.set_msgid(msgid);
    if (!data.empty()) {
        obj.set_msgdata(data);
    }
    return send_proto(&obj, enves);
}
int FilterData::send_proto(const ProtoStr c, const ProtoResponse s)
{
#ifdef USE_DEBUG
    if (s.timeout_sec.tv_sec != 0 && s.timeout_sec.tv_usec != 0) {
        auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
        int timerid = timermgr.add(std::make_shared<ResponseTimer>(
            s.timeout_sec,
            shared_from_this(),
            c.msgid,
            s.msgid));
        {
            std::lock_guard<std::mutex> lock(rtbmut_);
            auto it = response_timer_bind_.find(s.msgid);
            if (it == response_timer_bind_.end()) {
                std::list<int> ls;
                ls.emplace_back(timerid);
                response_timer_bind_.insert({ s.msgid, ls });
            } else {
                it->second.emplace_back(timerid);
            }
        }
    }
#endif
    {
        std::lock_guard<std::mutex> lock(pmbmut_);
        pmb_[s.msgid] = s.fun;
    }
    return send_proto(c.msgid, c.data, c.enves, c.extend);
}
int FilterData::send_proto(const ProtoMessage c, const ProtoResponse s)
{
#ifdef USE_DEBUG
    if (s.timeout_sec.tv_sec != 0 && s.timeout_sec.tv_usec != 0) {
        auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
        int timerid = timermgr.add(std::make_shared<ResponseTimer>(
            s.timeout_sec,
            shared_from_this(),
            c.msgid,
            s.msgid));
        {
            std::lock_guard<std::mutex> lock(rtbmut_);
            auto it = response_timer_bind_.find(s.msgid);
            if (it == response_timer_bind_.end()) {
                std::list<int> ls;
                ls.emplace_back(timerid);
                response_timer_bind_.insert({ s.msgid, ls });
            } else {
                it->second.emplace_back(timerid);
            }
        }
    }
#endif
    {
        std::lock_guard<std::mutex> lock(pmbmut_);
        pmb_[s.msgid] = s.fun;
    }
    return send_proto(c.msgid, c.data, c.enves, c.extend);
}

int FilterData::send_json(rapidjson::Document* doc, std::stack<Envelope>* enves) const
{
    if (enves != nullptr && enves->empty() == false) {
        doc->RemoveMember("rs");
        doc->AddMember("rs", rapidjson::Value(rapidjson::kArrayType), doc->GetAllocator());
    }
    std::stack<Envelope> new_enves;
    if (enves != nullptr)
        new_enves = *enves;
    while (new_enves.empty() == false) {
        Envelope& enve = new_enves.top();
        new_enves.pop();
        rapidjson::Value enve_json;
        enve_json.SetObject();
        enve_json.AddMember("fd", enve.fd, doc->GetAllocator());
        net::IPAddress ipaddr(&enve.addr);
        enve_json.AddMember("ip", rapidjson::StringRef(ipaddr.ip().c_str()), doc->GetAllocator());
        enve_json.AddMember("port", ipaddr.port(), doc->GetAllocator());
        (*doc)["rs"].PushBack(enve_json, doc->GetAllocator());
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    return filter_->send(buffer.GetString(), buffer.GetLength());
}
int FilterData::send_json(int msgid, rapidjson::Value* data, std::stack<Envelope>* enves) const
{
    rapidjson::Document root;
    root.SetObject();
    root.AddMember("msgid", msgid, root.GetAllocator());
    if (data != nullptr) {
        root.AddMember("msgdata", *data, root.GetAllocator());
    }
    return send_json(&root, enves);
}
int FilterData::send_json(const JsonDoc c, const JsonResponse s)
{
#ifdef USE_DEBUG
    if (s.timeout_sec.tv_sec != 0 && s.timeout_sec.tv_usec != 0) {
        int msgid = (*c.doc)["msgid"].GetInt();
        auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
        int timerid = timermgr.add(std::make_shared<ResponseTimer>(
            s.timeout_sec,
            shared_from_this(),
            msgid,
            s.msgid));
        {
            std::lock_guard<std::mutex> lock(rtbmut_);
            auto it = response_timer_bind_.find(s.msgid);
            if (it == response_timer_bind_.end()) {
                std::list<int> ls;
                ls.emplace_back(timerid);
                response_timer_bind_.insert({ s.msgid, ls });
            } else {
                it->second.emplace_back(timerid);
            }
        }
    }
#endif
    {
        std::lock_guard<std::mutex> lock(jmbmut_);
        jmb_[s.msgid] = s.fun;
    }
    return send_json(c.doc, c.enves);
}
int FilterData::send_json(const JsonValue c, const JsonResponse s)
{
#ifdef USE_DEBUG
    if (s.timeout_sec.tv_sec != 0 && s.timeout_sec.tv_usec != 0) {
        auto& timermgr = shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance();
        int timerid = timermgr.add(std::make_shared<ResponseTimer>(
            s.timeout_sec,
            shared_from_this(),
            c.msgid,
            s.msgid));
        {
            std::lock_guard<std::mutex> lock(rtbmut_);
            auto it = response_timer_bind_.find(s.msgid);
            if (it == response_timer_bind_.end()) {
                std::list<int> ls;
                ls.emplace_back(timerid);
                response_timer_bind_.insert({ s.msgid, ls });
            } else {
                it->second.emplace_back(timerid);
            }
        }
    }
#endif
    {
        std::lock_guard<std::mutex> lock(jmbmut_);
        jmb_[s.msgid] = s.fun;
    }
    return send_json(c.msgid, c.data, c.enves);
}

int FilterData::send_errcode(protocc::errnum code,
    const std::string& desc,
    std::stack<Envelope>* enves)
{
    if (pd_ == ProtoData::PROTOBUF) {
        protocc::errcode err;
        err.set_code(code);
        err.set_desc(desc);
        return send_proto(protocc::BroadcastMsgId::ERRCODE, &err, enves);
    } else if (pd_ == ProtoData::JSON) {
        rapidjson::Document root;
        root.SetObject();
        root.AddMember("msgid", protocc::BroadcastMsgId::ERRCODE, root.GetAllocator());
        rapidjson::Value err(rapidjson::kObjectType);
        err.AddMember("code", code, root.GetAllocator());
        err.AddMember("desc", rapidjson::StringRef(desc.c_str()), root.GetAllocator());
        root.AddMember("msgdata", err, root.GetAllocator());
        return send_json(&root, enves);
    } else {
        LOG_WARN << "不支持的封包类型";
    }
    return 0;
}
}
