#ifndef SHYNET_THREAD_ACCEPTTHREAD_H
#define SHYNET_THREAD_ACCEPTTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {

    class AcceptThread : public Thread {
    public:
        explicit AcceptThread(size_t index);
        ~AcceptThread();

        int event_tot() const
        {
            return eventTot_;
        }
        void set_event_tot(int v)
        {
            eventTot_ = v;
        }

        int run() override;
        int stop() override;
        int notify(const void* data, size_t len) const;
        void process(struct bufferevent* bev);

    private:
        std::shared_ptr<events::EventBuffer> pair_[2] = { 0 };
        std::shared_ptr<events::EventBase> base_ = nullptr;
        int eventTot_ = 0;
    };
}
}

#endif
