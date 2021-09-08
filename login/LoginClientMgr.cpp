#include "login/LoginClientMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "frmpub/LuaCallBackTask.h"

namespace login
{
	LoginClientMgr::LoginClientMgr()
	{
	}

	LoginClientMgr::~LoginClientMgr()
	{
	}

	void LoginClientMgr::add(int k, std::shared_ptr<LoginClient> v)
	{
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });

		//通知lua的onAccept函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnAcceptTask<LoginClient>>(v));
	}

	bool LoginClientMgr::remove(int k)
	{
		std::lock_guard<std::mutex> lock(clis_mutex_);
		if (clis_.erase(k) > 0) {

			//通知lua的onClose函数
			shynet::Singleton<lua::LuaEngine>::get_instance().append(
				std::make_shared<frmpub::OnCloseTask>(k));

			return true;
		}
		return false;
	}

	std::shared_ptr<LoginClient> LoginClientMgr::find(int k)
	{
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::shared_ptr<LoginClient> LoginClientMgr::find_from_sid(const std::string& sid) const
	{
		std::lock_guard<std::mutex> lock(clis_mutex_);
		for (auto& it : clis_)
		{
			if (std::to_string(it.second->sif().sid()) == sid)
			{
				return it.second;
			}
		}
		return nullptr;
	}

	std::unordered_map<int, std::shared_ptr<LoginClient>> LoginClientMgr::clis() const
	{
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}

	const net::IPAddress& LoginClientMgr::listen_addr() const
	{
		return listen_addr_;
	}
	void LoginClientMgr::listen_addr(const net::IPAddress& addr)
	{
		listen_addr_ = addr;
	}
}
