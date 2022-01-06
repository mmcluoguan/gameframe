#include "frmpub/frmluawrapper.h"
#include "frmpub/client.h"
#include "frmpub/connector.h"
#include "frmpub/luatimertask.h"

namespace frmpub {
void FrmLuaWrapper::init(kaguya::State& state)
{
    state("package.path=package.path ..\";./?.lua;./lualibs/?/?.lua;./lualibs/?.lua\"");
    state("package.cpath=package.cpath ..\";./luabin/?.so;./luabin/clibs53/?.so\"");
    lua::LuaWrapper::init(state);

    state["schedule_timer"] = kaguya::function([](int millisecond, bool repeat, std::string funname = "") {
        int second = millisecond / 1000;
        int milli = millisecond - second * 1000;
        return shynet::utils::Singleton<LuaTimerMgr>::instance().add({ second, milli * 1000 }, repeat, funname);
    });

    state["cancel_timer"] = kaguya::function([](int timerid) {
        shynet::utils::Singleton<LuaTimerMgr>::instance().remove(timerid);
    });

    state["Envelope_CPP"].setClass(kaguya::UserdataMetatable<FilterData::Envelope>()
                                       .setConstructors<FilterData::Envelope()>()
                                       .addOverloadedFunctions("fd",
                                           &FilterData::Envelope::set_fd,
                                           &FilterData::Envelope::get_fd)
                                       .addOverloadedFunctions("addr",
                                           &FilterData::Envelope::set_addr,
                                           &FilterData::Envelope::get_addr));

    using StackEnvelope = std::stack<FilterData::Envelope>;
    state["StackEnvelope_CPP"].setClass(kaguya::UserdataMetatable<StackEnvelope>()
                                            .setConstructors<StackEnvelope()>()
                                            .addFunction("push",
                                                static_cast<void (StackEnvelope::*)(const FilterData::Envelope&)>(&StackEnvelope::push))
                                            .addFunction("pop", &StackEnvelope::pop)
                                            .addFunction("size", &StackEnvelope::size)
                                            .addFunction("copy",
                                                static_cast<StackEnvelope& (StackEnvelope::*)(const StackEnvelope&)>(&StackEnvelope::operator=))
                                            .addFunction("top",
                                                static_cast<const FilterData::Envelope& (StackEnvelope::*)() const>(&StackEnvelope::top)));

    state["FilterData_CPP"].setClass(kaguya::UserdataMetatable<FilterData>()
                                         .addFunction("send_proto", static_cast<int (FilterData::*)(int, const std::string, std::stack<FilterData::Envelope>*, const std::string*) const>(&FilterData::send_proto)));

    state["ServerInfo_CPP"].setClass(kaguya::UserdataMetatable<protocc::ServerInfo>()
                                         .addFunction("ip", &protocc::ServerInfo::ip)
                                         .addFunction("port", &protocc::ServerInfo::port)
                                         .addFunction("sid", &protocc::ServerInfo::sid)
                                         .addFunction("name", &protocc::ServerInfo::name)
                                         .addFunction("st", &protocc::ServerInfo::st));

    state["Client_CPP"].setClass(kaguya::UserdataMetatable<Client, kaguya::MultipleBase<net::AcceptNewFd, FilterData>>()
                                     .addFunction("sif", static_cast<protocc::ServerInfo (Client::*)() const>(&Client::sif)));

    state["Connector_CPP"].setClass(kaguya::UserdataMetatable<Connector, kaguya::MultipleBase<net::ConnectEvent, FilterData>>()
                                        .addFunction("name", &Connector::name));
}
}
