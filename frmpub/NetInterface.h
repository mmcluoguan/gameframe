#pragma once
#include <json/json.h>
#include "frmpub/protocc/common.pb.h"
#include "shynet/protocol/FilterProces.h"

namespace frmpub {
	class NetInterface : public protocol::FilterProces {
	public:
		NetInterface(std::shared_ptr<events::EventBuffer> iobuf, UseProto useproto, Identity ident);
		~NetInterface();

		int protobuf_handle(const char* original_data, size_t len, std::stack<protocol::FilterProces::Envelope>& enves) override;
		int json_handle(const Json::Value& doc, std::stack<protocol::FilterProces::Envelope>& enves) override;
		int native_handle(const char* original_data, size_t datalen) override;

		virtual int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<protocol::FilterProces::Envelope>& enves) = 0;
		virtual int input_handle(const Json::Value& obj, std::stack<protocol::FilterProces::Envelope>& enves) = 0;
		virtual int input_handle(const char* original_data, size_t datalen) = 0;

		int send_proto(const protocc::CommonObject* data,
			std::stack<protocol::FilterProces::Envelope>* enves = nullptr) const;
		int send_proto(int msgid, const google::protobuf::Message* data = nullptr,
			std::stack<protocol::FilterProces::Envelope>* enves = nullptr) const;

		int send_json(Json::Value* data,
			std::stack<protocol::FilterProces::Envelope>* enves = nullptr) const;
		int send_json(int msgid, const Json::Value* data = nullptr,
			std::stack<protocol::FilterProces::Envelope>* enves = nullptr) const;

		void ping() const;
		void pong() const;
	private:
	};
}
