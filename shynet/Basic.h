#pragma once

namespace shynet {
	namespace utils {};
	namespace events {};
	namespace net {};
	namespace pool {};
	namespace crypto {};
	namespace thread {};
	namespace lua {};
	namespace task {};
	namespace protocol {
		enum class Step {
			UNINIT = 0,
			INIT,
			Parse,
		};
	};

	/// <summary>
	/// 禁止拷贝
	/// </summary>
	class Nocopy {
	public:
		Nocopy(const Nocopy&) = delete;
		Nocopy(Nocopy&) = delete;
		Nocopy(Nocopy&&) = delete;
		Nocopy& operator=(const Nocopy&) = delete;
		Nocopy& operator=(Nocopy&) = delete;
		Nocopy& operator=(Nocopy&&) = delete;
		virtual ~Nocopy() = default;
	protected:
		Nocopy() = default;
	};
}

namespace events = shynet::events;
namespace net = shynet::net;
namespace pool = shynet::pool;
namespace crypto = shynet::crypto;
namespace protocol = shynet::protocol;
namespace thread = shynet::thread;
namespace lua = shynet::lua;
namespace task = shynet::task;
