#include "frmpub/FrmLuaWrapper.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "frmpub/Client.h"
#include "frmpub/Connector.h"
#include "frmpub/LuaTimer.h"
#include "frmpub/protocc/common.pb.h"

namespace frmpub {
	FrmLuaWrapper::FrmLuaWrapper() {
	}
	FrmLuaWrapper::~FrmLuaWrapper() {
	}
	void FrmLuaWrapper::init(kaguya::State& state) {
		lua::LuaWrapper::init(state);

		state["schedule_timer"] = kaguya::function([](int millisecond, bool repeat) {
			int second = millisecond / 1000;
			int milli = millisecond - second * 1000;
			return shynet::Singleton<LuaTimerMgr>::instance().add({ second,milli * 1000 }, repeat);
			});

		state["cancel_timer"] = kaguya::function([](int timerid) {
			shynet::Singleton<LuaTimerMgr>::instance().remove(timerid);
			});

		state["Envelope_CPP"].setClass(kaguya::UserdataMetatable<FilterData::Envelope>()
			.setConstructors<FilterData::Envelope()>()
			.addOverloadedFunctions("fd",
				&FilterData::Envelope::set_fd,
				&FilterData::Envelope::get_fd)
			.addOverloadedFunctions("addr",
				&FilterData::Envelope::set_addr,
				&FilterData::Envelope::get_addr)
		);

		using StackEnvelope = std::stack<FilterData::Envelope>;
		state["StackEnvelope_CPP"].setClass(kaguya::UserdataMetatable<StackEnvelope>()
			.addFunction("push",
				static_cast<void (StackEnvelope::*)(const FilterData::Envelope&)>(&StackEnvelope::push))
			.addFunction("pop", &StackEnvelope::pop)
			.addFunction("size", &StackEnvelope::size)
			.addFunction("top",
				static_cast<const FilterData::Envelope & (StackEnvelope::*)() const>(&StackEnvelope::top))
		);

		state["FilterData_CPP"].setClass(kaguya::UserdataMetatable<FilterData>()
			.addFunction("send_proto", static_cast<int (FilterData::*)(int, const std::string, std::stack<FilterData::Envelope>*, const std::string*) const>(&FilterData::send_proto))
		);

		state["ServerInfo_CPP"].setClass(kaguya::UserdataMetatable<protocc::ServerInfo>()
			.addFunction("ip", &protocc::ServerInfo::ip)
			.addFunction("port", &protocc::ServerInfo::port)
			.addFunction("sid", &protocc::ServerInfo::sid)
			.addFunction("name", &protocc::ServerInfo::name)
			.addFunction("st", &protocc::ServerInfo::st)
		);

		state["Client_CPP"].setClass(kaguya::UserdataMetatable<Client, kaguya::MultipleBase<net::AcceptNewFd, FilterData>>()
			.addFunction("sif", static_cast<protocc::ServerInfo(Client::*)() const>(&Client::sif))
		);

		state["Connector_CPP"].setClass(kaguya::UserdataMetatable<Connector, kaguya::MultipleBase<net::ConnectEvent, FilterData>>()
			.addFunction("name", &Connector::name)
		);
	}
}
