#include "shynet/events/eventconfig.h"
#include "shynet/utils/logger.h"

namespace shynet {
namespace events {
    EventConfig::EventConfig()
    {
        config_ = event_config_new();
        if (config_ == nullptr) {
            THROW_EXCEPTION("call event_config_new");
        }
    }
    EventConfig::~EventConfig()
    {
        if (config_ != nullptr)
            event_config_free(config_);
    }

    event_config* EventConfig::config() const
    {
        return config_;
    }

    int EventConfig::setfeatures(event_method_feature feature) const
    {
        return event_config_require_features(config_, feature);
    }
    int EventConfig::setflag(event_base_config_flag flag) const
    {
        return event_config_set_flag(config_, flag);
    }
}
}
