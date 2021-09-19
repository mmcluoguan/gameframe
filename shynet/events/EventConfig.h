#pragma once
#include "shynet/Basic.h"
#include <event2/event.h>

namespace shynet
{
	namespace events
	{

		/// <summary>
		/// 反应堆配置
		/// </summary>
		class EventConfig final : public Nocopy
		{
		public:
			EventConfig();
			~EventConfig();

			/// <summary>
			/// 设置特性
			/// </summary>
			/// <param name="feature"></param>
			/// <returns>0成功,-1失败</returns>
			int setfeatures(event_method_feature feature) const;
			/// <summary>
			/// 设置标识
			/// </summary>
			/// <param name="flag"></param>
			/// <returns></returns>
			int setflag(event_base_config_flag flag) const;

			event_config* config() const;
		private:
			event_config* config_ = nullptr;
		};
	}
}
