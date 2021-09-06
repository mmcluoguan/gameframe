#include "frmpub/NetInterface.h"
#include <memory>
#include "shynet/net/IPAddress.h"
#include "shynet/Utility.h"
#include "shynet/Logger.h"
#include "Basic.h"

namespace frmpub {
	NetInterface::NetInterface(std::shared_ptr<events::EventBuffer> iobuf, UseProto useproto, Identity ident) 
		: protocol::FilterProces(iobuf,useproto,ident) {
		GOBJ++;
		LOG_TRACE << "NetInterface:" << GOBJ;
	}
	NetInterface::~NetInterface() {
		GOBJ--;
		LOG_TRACE << "NetInterface:" << GOBJ;
	}
	int NetInterface::protobuf_handle(const char* original_data, size_t len, std::stack<protocol::FilterProces::Envelope>& enves) {
		std::shared_ptr<protocc::CommonObject> obj(new protocc::CommonObject);
		if (obj->ParseFromArray(original_data, (int)len) == true) {
			if (obj->msgid() == protocc::CmdMsgId::PING) {
				pong();
				return 0;
			}
			else if (obj->msgid() == protocc::CmdMsgId::PONG) {
				return 0;
			}
			else {
				return input_handle(obj, enves);
			}
		}
		else {
			LOG_WARN << "protocc::CommonObject 反序列化失败";
			return -1;
		}
	}

	int NetInterface::json_handle(const Json::Value& doc, std::stack<protocol::FilterProces::Envelope>& enves) {
		if (doc["msgid"].isUInt()) {
			if (doc["msgid"].asUInt() == protocc::CmdMsgId::PING) {
				ping();
				return 0;
			}
			else if (doc["msgid"].asUInt() == protocc::CmdMsgId::PONG) {
				return 0;
			}
			else {
				return input_handle(doc, enves);
			}
		}
		else {
			LOG_WARN << "json msgid的值类型不是unsigned int";
			return -1;
		}
	}

	int NetInterface::native_handle(const char* original_data, size_t datalen) {
		return input_handle(original_data, datalen);
	}

	int NetInterface::send_proto(const protocc::CommonObject* data, std::stack<protocol::FilterProces::Envelope>* enves) const {
		std::string original_data = data->SerializePartialAsString();
		size_t total_data_len = original_data.length();

		std::unique_ptr<char[]> data_ptr;
		const char* data_buffer = original_data.c_str();
		if (enves != nullptr && enves->empty() == false) {
			char head_zf[] = "zf";
			size_t envelope_size = sizeof(uint32_t) + sizeof(sockaddr_storage);
			size_t head_zf_size =
				enves->size() * envelope_size + enves->size() * strlen(head_zf);
			total_data_len += head_zf_size;
			data_ptr.reset(new char[total_data_len]);
			size_t pos = 0;
			while (enves != nullptr && enves->empty() == false) {
				protocol::FilterProces::Envelope enve = enves->top();
				enves->pop();
				memcpy(data_ptr.get() + pos, head_zf, strlen(head_zf));
				pos += strlen(head_zf);
				uint32_t fd = htonl(enve.fd);
				memcpy(data_ptr.get() + pos, &fd, sizeof(fd));
				pos += sizeof(fd);
				memcpy(data_ptr.get() + pos, &enve.addr, sizeof(enve.addr));
				pos += sizeof(enve.addr);
			}
			memcpy(data_ptr.get() + pos, original_data.c_str(), original_data.length());
			data_buffer = data_ptr.get();
		}

		return send(data_buffer, total_data_len);
	}
	int NetInterface::send_proto(int msgid, const google::protobuf::Message* data, std::stack<protocol::FilterProces::Envelope>* enves) const {
		protocc::CommonObject obj;
		obj.set_msgid(msgid);
		if (data != nullptr) {
			obj.set_msgdata(data->SerializePartialAsString());
		}
		if (msgid != protocc::NEW_RECHARGE_S)
			LOG_DEBUG << "send:" << Basic::client_msgname(msgid);
		return send_proto(&obj, enves);
	}

	int NetInterface::send_json(Json::Value* data, std::stack<protocol::FilterProces::Envelope>* enves) const {
		while (enves != nullptr && enves->empty() == false) {
			protocol::FilterProces::Envelope enve = enves->top();
			enves->pop();
			Json::Value enve_json;
			enve_json["fd"] = enve.fd;
			net::IPAddress ipaddr(&enve.addr);
			enve_json["ip"] = ipaddr.ip();
			enve_json["port"] = ipaddr.port();
			(*data)["zf"].append(enve_json);
		}
		Json::FastWriter fw;
		return send(fw.write(data), enves);
	}
	int NetInterface::send_json(int msgid, const Json::Value* data, std::stack<protocol::FilterProces::Envelope>* enves) const {
		Json::Value root;
		root["msgid"] = msgid;
		if (data != nullptr) {
			root["msgdata"] = *data;
		}
		return send_json(&root, enves);
	}
	void NetInterface::ping() const {
		if (useproto().pd == protocol::FilterProces::ProtoData::PROTOBUF) {
			protocc::CommonObject ping;
			ping.set_msgid(protocc::CmdMsgId::PING);
			send_proto(&ping);
		}
		else if (useproto().pd == protocol::FilterProces::ProtoData::JSON) {
			Json::Value ping;
			ping["msgid"] = (int)protocc::CmdMsgId::PING;
			send_json(&ping);
		}
	}
	void NetInterface::pong() const {
		if (useproto().pd == protocol::FilterProces::ProtoData::PROTOBUF) {
			protocc::CommonObject pong;
			pong.set_msgid(protocc::CmdMsgId::PONG);
			send_proto(&pong);
		}
		else if (useproto().pd == protocol::FilterProces::ProtoData::JSON) {
			Json::Value pong;
			pong["msgid"] = (int)protocc::CmdMsgId::PONG;
			send_json(&pong);
		}
	}
}
