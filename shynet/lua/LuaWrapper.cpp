#include "shynet/lua/LuaWrapper.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ListenEvent.h"
#include "shynet/net/ListenReactorMgr.h"
#include "shynet/net/AcceptNewFd.h"
#include "shynet/net/ConnectEvent.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"

namespace shynet {
	namespace lua {
		LuaWrapper::LuaWrapper() {
		}
		LuaWrapper::~LuaWrapper() {
		}
		void LuaWrapper::init(kaguya::State& state) {
			state.setErrorHandler([](int errCode, const char* szError) {
				//LOG_WARN << "LuaErrCode:" << errCode;
				LOG_WARN << szError;
				});

			//注册函数
			state["log"] = kaguya::function([](kaguya::VariadicArgType args) {
				std::ostringstream stream;
				for (auto v : args) {
					stream << v.get<std::string>() << " ";
				}
				LOG_LUA << stream.str();
				});

			//注册c++类			
			state["IPAddress_CPP"].setClass(kaguya::UserdataMetatable<net::IPAddress>()
				.addFunction("ip", &net::IPAddress::ip)
				.addFunction("port", &net::IPAddress::port)
			);

			state["ListenEvent_CPP"].setClass(kaguya::UserdataMetatable<net::ListenEvent>()
				.addFunction("serverid",
					static_cast<int (net::ListenEvent::*)() const>(&net::ListenEvent::serverid))
				.addFunction("stop", &net::ListenEvent::stop)
				.addFunction("listenfd", &net::ListenEvent::listenfd)
				.addFunction("listenaddr", &net::ListenEvent::listenaddr)
			);

			state["ListenReactorMgr_CPP"].setClass(kaguya::UserdataMetatable<net::ListenReactorMgr>()
				.addFunction("find", &net::ListenReactorMgr::find)
			);

			state["FilterProces_CPP"].setClass(kaguya::UserdataMetatable<protocol::FilterProces>()
				.addFunction("ident", &protocol::FilterProces::ident)
			);

			state["AcceptNewFd_CPP"].setClass(kaguya::UserdataMetatable<net::AcceptNewFd, protocol::FilterProces>()
				.addFunction("remote_addr", &net::AcceptNewFd::remote_addr)
				.addFunction("listen_addr", &net::AcceptNewFd::listen_addr)
				.addFunction("fd", &net::AcceptNewFd::fd)
				.addFunction("close", &net::AcceptNewFd::close)
			);

			state["ConnectEvent_CPP"].setClass(kaguya::UserdataMetatable<net::ConnectEvent, protocol::FilterProces>()
				.addFunction("connectid", static_cast<int (net::ConnectEvent::*)() const>(&net::ConnectEvent::connectid))
				.addFunction("connect_addr", &net::ConnectEvent::connect_addr)
				.addFunction("fd", &net::ConnectEvent::fd)
				.addFunction("close", &net::ConnectEvent::close)
			);
		}
	}
}
