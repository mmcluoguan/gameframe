#include "gate/ConnectorMgr.h"
#include "shynet/net/ConnectReactorMgr.h"

namespace gate {
	ConnectorMgr::ConnectorMgr() {
	}
	ConnectorMgr::~ConnectorMgr() {
	}
	int ConnectorMgr::sid_conv_connect_id(int sid) {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		for (const auto& it : connect_datas_) {
			if (it.second.sif.sid() == sid) {
				return it.second.connect_id;
			}
		}
		return 0;
	}

	void ConnectorMgr::add_new_connect_data(int connect_id, protocc::ServerInfo sif) {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		ConnectData data;
		data.connect_id = connect_id;
		data.sif = sif;
		connect_datas_[connect_id] = data;
	}

	ConnectorMgr::ConnectData* ConnectorMgr::find_connect_data(int connect_id)
	{
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		auto it = connect_datas_.find(connect_id);
		if (it != connect_datas_.end()) {
			return &it->second;
		}
		return nullptr;
	}

	std::shared_ptr<LoginConnector> ConnectorMgr::select_login(int login_connect_id) {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		std::shared_ptr<LoginConnector> login;
		auto iter_data = connect_datas_.find(login_connect_id);
		ConnectData* data = nullptr;
		if (iter_data == connect_datas_.end()) {
			//如果指定login_id找不到,默认选择connect_num值最小的登录服务器
			int min = -1;
			int target_login_id = 0;
			for (auto& it : connect_datas_) {
				if (it.second.sif.st() == protocc::ServerType::LOGIN) {
					if (min == -1) {
						min = it.second.connect_num;
						target_login_id = it.second.connect_id;
						data = &(it.second);
					}
					else if (it.second.connect_num < min) {
						min = it.second.connect_num;
						target_login_id = it.second.connect_id;
						data = &(it.second);
					}
				}
			}
			login = std::dynamic_pointer_cast<LoginConnector>(
				shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(target_login_id));
			if (data != nullptr) {

				data->connect_num++;
			}
		}
		else {
			login = std::dynamic_pointer_cast<LoginConnector>(
				shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(login_connect_id));
			iter_data->second.connect_num++;
		}
		return login;
	}

	bool ConnectorMgr::remove(int connect_id) {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		return connect_datas_.erase(connect_id) > 0 ? true : false;
	}

	bool ConnectorMgr::reduce_count(int connect_id) {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		for (auto& it : connect_datas_) {
			if (it.second.connect_id == connect_id) {
				it.second.connect_num--;
				if (it.second.connect_num < 0)
					it.second.connect_num = 0;
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<WorldConnector> ConnectorMgr::world_connector()  {
		if (worldctor_ids_.begin() != worldctor_ids_.end())
		{
			return find_worldctor(*worldctor_ids_.begin());
		}
		return nullptr;
	}

	void ConnectorMgr::add_worldctor(int connectid)
	{
		std::lock_guard<std::mutex> lock(worldctor_ids_mtx_);
		worldctor_ids_.push_back(connectid);
	}
	void ConnectorMgr::remove_worldctor(int connectid)
	{
		std::lock_guard<std::mutex> lock(worldctor_ids_mtx_);
		worldctor_ids_.remove(connectid);
	}
	std::shared_ptr<WorldConnector> ConnectorMgr::find_worldctor(int connectid)
	{
		return std::dynamic_pointer_cast<WorldConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(connectid));
	}

	std::shared_ptr<LoginConnector> ConnectorMgr::login_connector(int login_connect_id) const {
		return std::dynamic_pointer_cast<LoginConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(login_connect_id));
	}

	std::shared_ptr<GameConnector> ConnectorMgr::game_connector(int game_connect_id) const {
		return std::dynamic_pointer_cast<GameConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(game_connect_id));
	}

	std::unordered_map<int, ConnectorMgr::ConnectData> ConnectorMgr::connect_datas() const {
		std::lock_guard<std::mutex> lock(connect_data_mutex_);
		return connect_datas_;
	}
}
