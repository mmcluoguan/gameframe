#ifndef SHYNET_PROTOCOL_FILTERPROCES_H
#define SHYNET_PROTOCOL_FILTERPROCES_H

#include "shynet/events/EventBuffer.h"
#include "shynet/net/IPAddress.h"
#include "shynet/protocol/Http.h"
#include "shynet/protocol/Tcpip.h"
#include "shynet/protocol/WebSocket.h"

namespace shynet {
	namespace protocol {
		/// <summary>
		/// 协议过滤器
		/// </summary>
		class FilterProces : public Nocopy {
		public:
			/// <summary>
			/// 身份标识
			/// </summary>
			enum class Identity {
				ACCEPTOR,  //接收身份
				CONNECTOR, //连接身份
			};

			/// <summary>
			/// 协议类型
			/// </summary>
			enum class ProtoType {
				SHY,  //自定义协议 {/r/n + uint32 + len}
				HTTP, //http协议
				WEBSOCKET, //websocket协议
			};


			/// <summary>
			/// 协议过滤器
			/// </summary>
			/// <param name="iobuf">io读写缓冲区</param>
			/// <param name="pt">协议类型</param>
			/// <param name="ident">身份标识</param>
			FilterProces(std::shared_ptr<events::EventBuffer> iobuf, ProtoType pt, Identity ident);
			~FilterProces();

			/// <summary>
			/// 连接服务器成功回调
			/// </summary>
			virtual void complete() {
			}

			/*
			* 消息数据封包处理
			*/
			virtual int message_handle(char* original_data, size_t datalen) = 0;

			/*
			* 发送数据
			*/
			int send(const char* data, size_t datalen) const;
			int send(std::string data) const;

			/*
			* 发送ping包
			*/
			int ping() const;

			ProtoType pt() const {
				return pt_;
			}

			Identity ident() const;

			/// <summary>
			/// 接收缓冲区最大值
			/// </summary>
			const size_t max_reve_buf_size = 4 * 1024 * 1024;
			/// <summary>
			/// 单包最大值(发送时分包)
			/// </summary>
			const size_t max_single_buf_size = 1024 * 64;

			/*
			* 获取设置io缓冲区
			*/
			void iobuf(std::shared_ptr<events::EventBuffer> iobuf) {
				iobuf_ = iobuf;
			}
			std::shared_ptr<events::EventBuffer> iobuf() const {
				return iobuf_;
			}

		protected:
			/// <summary>
			/// 解析数据流协议
			/// </summary>
			/// <returns>0正常处理,-1服务器主动断开连接,-2客户端断开连接</returns>
			int process();
			int request_handshake() const;

		private:
			std::shared_ptr<events::EventBuffer> iobuf_;
			ProtoType pt_ = ProtoType::SHY;
			Identity ident_;
			std::unique_ptr<Tcpip> tcpip_;
			std::unique_ptr<Http> http_;
			std::unique_ptr<WebSocket> websocket_;
		};
	}
}

#endif
