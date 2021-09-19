#pragma once
#include "frmpub/Basic.h"
#include "shynet/protocol/FilterProces.h"
#include <rapidjson/document.h>

namespace frmpub {
	/// <summary>
	/// 消息数据封包过滤
	/// </summary>
	class FilterData : public shynet::Nocopy {
	public:
		/// <summary>
		/// 数据封包类型
		/// </summary>
		enum class ProtoData {
			PROTOBUF = 0,
			JSON,
			NATIVE,
		};

		/// <summary>
		/// 消息信封
		/// </summary>
		struct Envelope {
			uint32_t fd;
			struct sockaddr_storage addr; //消息源地址

			void set_addr(const std::shared_ptr<net::IPAddress> ipaddr) {
				addr = *ipaddr->sockaddr();
			}

			net::IPAddress get_addr() const {
				return net::IPAddress(&addr);
			}

			void set_fd(uint32_t value) {
				fd = value;
			}

			uint32_t get_fd() const {
				return fd;
			}
		};

		/// <summary>
		/// 消息数据封包过滤
		/// </summary>
		/// <param name="pd">数据封包类型</param>
		FilterData(ProtoData pd);
		~FilterData();

		/*
		* 消息数据封包处理
		*/
		int message_handle(char* original_data, size_t datalen);

		/*
		* 处理protobuf数据封包
		*/
		virtual int input_handle(const std::shared_ptr<protocc::CommonObject> obj,
			std::shared_ptr<std::stack<Envelope>> enves) {
			return 0;
		};
		/*
		* 处理json数据封包
		*/
		virtual int input_handle(const std::shared_ptr<rapidjson::Document> obj,
			std::shared_ptr<std::stack<Envelope>> enves) {
			return 0;
		}
		/*
		* 处理原始数据封包
		*/
		virtual int input_handle(const char* original_data, size_t datalen,
			std::shared_ptr<std::stack<Envelope>> enves) {
			return 0;
		};

		/*
		* 发送proto数据封包
		*/
		int send_proto(protocc::CommonObject* data,
			std::stack<Envelope>* enves = nullptr) const;
		int send_proto(int msgid, const google::protobuf::Message* data = nullptr,
			std::stack<Envelope>* enves = nullptr, const std::string* extend = nullptr) const;
		int send_proto(int msgid, const std::string data,
			std::stack<Envelope>* enves = nullptr, const std::string* extend = nullptr) const;

		/*
		* 发送json数据封包
		*/
		int send_json(rapidjson::Document* doc,
			std::stack<Envelope>* enves = nullptr) const;
		int send_json(int msgid, rapidjson::Value* data = nullptr,
			std::stack<Envelope>* enves = nullptr) const;

		/*
		* 服务器通用错误信息
		*/
		int send_errcode(protocc::errnum code, const std::string& desc,
			std::stack<Envelope>* enves = nullptr);
	protected:
		int native_handle(const char* original_data, size_t datalen);
		int protobuf_handle(const char* original_data, size_t datalen);
		int json_handle(const char* original_data, size_t datalen);

		using ProtoMsgBind = std::function<int(std::shared_ptr<protocc::CommonObject> obj,
			std::shared_ptr<std::stack<Envelope>> enves)> ;
		std::unordered_map<int, ProtoMsgBind> pmb_;

		using JsonMsgBind = std::function<int(std::shared_ptr<rapidjson::Document> doc,
			std::shared_ptr<std::stack<Envelope>> enves)> ;
		std::unordered_map<int, JsonMsgBind> jmb_;

		using NativeMsgBind = std::function<int(const char* original_data, size_t datalen,
			std::shared_ptr<std::stack<Envelope>> enves)> ;
		std::unordered_map<int, NativeMsgBind> nmb_;

		protocol::FilterProces* filter_ = nullptr;
	private:
		ProtoData pd_ = ProtoData::NATIVE;
	};
}
