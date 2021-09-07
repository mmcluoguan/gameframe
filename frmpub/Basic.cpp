#include "frmpub/Basic.h"
#include "shynet/Utility.h"
#include "shynet/Logger.h"

namespace frmpub {
	std::unordered_map<int, std::string> Basic::internal_msgname_
	{
		{ (int)protocc::BroadcastMsgId::ERRCODE,"ERRCODE" },

		{ (int)protocc::InternalMsgId::REGISTER_WORLD_DBVISIT_C,"REGISTER_WORLD_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::REGISTER_WORLD_DBVISIT_S,"REGISTER_WORLD_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::SERONLINE_WORLD_GATE_G,"SERONLINE_WORLD_GATE_G" },

		{ (int)protocc::InternalMsgId::REGISTER_LOGIN_DBVISIT_C,"REGISTER_LOGIN_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::REGISTER_LOGIN_DBVISIT_C,"REGISTER_LOGIN_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::REGISTER_LOGIN_DBVISIT_C,"REGISTER_LOGIN_WORLD_C" },
		{ (int)protocc::InternalMsgId::REGISTER_LOGIN_DBVISIT_C,"REGISTER_LOGIN_WORLD_S" },
		{ (int)protocc::InternalMsgId::GAMESID_LOGIN_WORLD_C,"GAMESID_LOGIN_WORLD_C" },
		{ (int)protocc::InternalMsgId::GAMESID_LOGIN_WORLD_S,"GAMESID_LOGIN_WORLD_S" },

		{ (int)protocc::InternalMsgId::REGISTER_GAME_DBVISIT_C,"REGISTER_GAME_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::REGISTER_GAME_DBVISIT_S,"REGISTER_GAME_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::REGISTER_GAME_WORLD_C,"REGISTER_GAME_WORLD_C" },
		{ (int)protocc::InternalMsgId::REGISTER_GAME_WORLD_S,"REGISTER_GAME_WORLD_S" },

		{ (int)protocc::InternalMsgId::REGISTER_GATE_WORLD_C,"REGISTER_GATE_WORLD_C" },
		{ (int)protocc::InternalMsgId::REGISTER_GATE_WORLD_S,"REGISTER_GATE_WORLD_S" },
		{ (int)protocc::InternalMsgId::REGISTER_GATE_LOGIN_C,"REGISTER_GATE_LOGIN_C" },
		{ (int)protocc::InternalMsgId::REGISTER_GATE_LOGIN_S,"REGISTER_GATE_LOGIN_S" },
		{ (int)protocc::InternalMsgId::REGISTER_GATE_GAME_C,"REGISTER_GATE_GAME_C" },
		{ (int)protocc::InternalMsgId::REGISTER_GATE_GAME_S,"REGISTER_GATE_GAME_S" },
		{ (int)protocc::InternalMsgId::CLIOFFLINE_GATE_ALL_C,"CLIOFFLINE_GATE_ALL_C" },

		{ (int)protocc::InternalMsgId::LOADDATA_FROM_DBVISIT_C,"LOADDATA_FROM_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::LOADDATA_FROM_DBVISIT_S,"LOADDATA_FROM_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::LOADDATA_MORE_FROM_DBVISIT_C,"LOADDATA_MORE_FROM_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::LOADDATA_MORE_FROM_DBVISIT_S,"LOADDATA_MORE_FROM_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::INSERTDATA_TO_DBVISIT_C,"INSERTDATA_TO_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::INSERTDATA_TO_DBVISIT_S,"INSERTDATA_TO_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::UPDATA_TO_DBVISIT_C,"UPDATA_TO_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::UPDATA_TO_DBVISIT_S,"UPDATA_TO_DBVISIT_S" },
		{ (int)protocc::InternalMsgId::DELETEDATA_TO_DBVISIT_C,"DELETEDATA_TO_DBVISIT_C" },
		{ (int)protocc::InternalMsgId::DELETEDATA_TO_DBVISIT_S,"DELETEDATA_TO_DBVISIT_S" },
	};

	std::unordered_map<int, std::string> Basic::client_msgname_
	{
		{ (int)protocc::ClientMsgId::CLIENT_GATE_BEGIN,"CLIENT_GATE_BEGIN" },
		{ (int)protocc::ClientMsgId::SERVERLIST_CLIENT_GATE_C,"SERVERLIST_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::SERVERLIST_CLIENT_GATE_S,"SERVERLIST_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::SELECTSERVER_CLIENT_GATE_C,"SELECTSERVER_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::SELECTSERVER_CLIENT_GATE_S,"SELECTSERVER_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::REPEATLOGIN_CLIENT_GATE_S,"REPEATLOGIN_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::CLIENT_GATE_END,"CLIENT_GATE_END" },

		{ (int)protocc::ClientMsgId::CLIENT_LOGIN_BEGIN,"CLIENT_LOGIN_BEGIN" },
		{ (int)protocc::ClientMsgId::LOGIN_CLIENT_GATE_C,"LOGIN_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::LOGIN_CLIENT_GATE_S,"LOGIN_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::RECONNECT_CLIENT_GATE_C,"RECONNECT_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::RECONNECT_CLIENT_GATE_S,"RECONNECT_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::CLIENT_LOGIN_END,"CLIENT_LOGIN_END" },

		{ (int)protocc::ClientMsgId::CLIENT_GAME_BEGIN,"CLIENT_GAME_BEGIN" },
		{ (int)protocc::ClientMsgId::CREATEROLE_CLIENT_GATE_C,"CREATEROLE_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::CREATEROLE_CLIENT_GATE_S,"CREATEROLE_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::LOADROLE_CLIENT_GATE_C,"LOADROLE_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::LOADROLE_CLIENT_GATE_S,"LOADROLE_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::SETLEVEL_CLIENT_GATE_C,"SETLEVEL_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::SETLEVEL_CLIENT_GATE_S,"SETLEVEL_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::LOADGOODS_CLIENT_GATE_C,"LOADGOODS_CLIENT_GATE_C" },
		{ (int)protocc::ClientMsgId::LOADGOODS_CLIENT_GATE_S,"LOADGOODS_CLIENT_GATE_S" },
		{ (int)protocc::ClientMsgId::CLIENT_GAME_END,"CLIENT_GAME_END" },


	};



	std::unordered_map<int, std::string> Basic::connect_type_
	{
		{(int)protocc::ServerType::UNKNOW,"UNKNOW"},
		{(int)protocc::ServerType::DBVISIT,"DBVISIT"},
		{(int)protocc::ServerType::WORLD,"WORLD"},
		{(int)protocc::ServerType::LOGIN,"LOGIN"},
		{(int)protocc::ServerType::GAME,"GAME"},
		{(int)protocc::ServerType::GATE,"GATE"},
	};

	std::string Basic::internal_msgname(int id) {
		const auto& it = internal_msgname_.find(id);
		if (it != internal_msgname_.end()) {
			return "(" + it->second + ":" + std::to_string(id) + ")";
		}
		return "(unknow:unknow)";
	}

	std::string Basic::client_msgname(int id) {
		const auto& it = client_msgname_.find(id);
		if (it != client_msgname_.end()) {
			return "(" + it->second + ":" + std::to_string(id) + ")";
		}
		return "(unknow:unknow)";
	}

	std::string Basic::msgname(int id) {
		std::string str = client_msgname(id);
		if (str == "(unknow:unknow)") {
			str = internal_msgname(id);
		}
		return str;
	}

	std::string Basic::connectname(protocc::ServerType st) {
		const auto& it = connect_type_.find(st);
		if (it != connect_type_.end()) {
			return "[" + it->second + "]";
		}
		return "[UNKNOW])";
	}
	std::string Sms::send(std::string url) {
		std::string response;
		CURL* curl = curl_easy_init();
		CURLcode res;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url  
		curl_easy_setopt(curl, CURLOPT_POST, 0); // get reqest 
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // set transport and time out time  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		// start request  
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (res != CURLE_OK) {
			LOG_WARN << curl_easy_strerror(res);
			return "";
		}
		return response;
	}
	size_t Sms::req_reply(void* ptr, size_t size, size_t nmemb, void* stream) {
		if (stream == NULL || ptr == NULL || size == 0)
			return 0;

		size_t realsize = size * nmemb;
		std::string* buffer = (std::string*)stream;
		if (buffer != NULL) {
			buffer->append((const char*)ptr, realsize);
		}
		return realsize;
	}
}
