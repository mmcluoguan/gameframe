#include "shynet/io/stdinhandler.h"
#include "shynet/3rd/readline/readline.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include <chrono>
#include <cstring>
#include <limits.h>
#include <readline/history.h>
#include <string_view>
#include <unistd.h>

extern int optind, opterr, optopt;
extern char* optarg;

namespace shynet {
namespace io {

    static std::vector<StdinHandler::OrderItem>* g_orders = nullptr;

    static void cb_linehandler(char* line)
    {
        if (line != nullptr && *line) {
            add_history(line);
            if (g_orders != nullptr) {
                char* order = shynet::utils::stringop::trim(line);
                char* argv[20] = { 0 };
                int argc = shynet::utils::stringop::split(order, " ", argv, 20);
                if (argc > 0) {
                    bool flag = false;
                    for (const auto& item : *g_orders) {
                        if (strcmp(argv[0], item.name) == 0) {
                            item.callback(item, argc, argv);
                            flag = true;
                        }
                    }
                    if (flag == false) {

                        LOG_INFO_BASE << "没有可执行的命令,可执行的命令列表";
                        for (const auto& it : *g_orders) {
                            LOG_INFO_BASE << "格式 : " << it.name << " " << it.argstr
                                          << "\t\t\t\t\t\t描述:" << it.desc;
                        }
                    }
                }
            }
        }
        free(line);
    }

    static char* command_generator(const char* text, int state)
    {
        static size_t list_index;
        if (!state) {
            list_index = 0;
        }
        if (g_orders != nullptr) {

            std::vector<StdinHandler::OrderItem>& items = *g_orders;
            for (; list_index < items.size();) {
                std::string_view view(items[list_index].name);
                if (view.find(text, 0) != std::string_view::npos) {
                    char* r = (char*)malloc(strlen(items[list_index].name) + 1);
                    strcpy(r, items[list_index].name);
                    list_index++;
                    return (r);
                }
                list_index++;
            }
        }
        return nullptr;
    }

    static char** gmman_completion(const char* text, int start, int end)
    {
        char** matches;

        matches = (char**)NULL;

        if (start == 0)
            matches = rl_completion_matches(text, command_generator);

        rl_attempted_completion_over = 1;
        return (matches);
    }

    StdinHandler::StdinHandler(std::shared_ptr<events::EventBase> base)
        : events::EventHandler(base, STDIN_FILENO, EV_READ | EV_PERSIST)
    {
        rl_catch_signals = 0;
        rl_callback_handler_install(nullptr, cb_linehandler);
        rl_attempted_completion_function = gmman_completion;
        orderitems_.push_back({ "quit", ":", "退出",
            std::bind(&StdinHandler::quit_order, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

        orderitems_.push_back({ "info", ":", "显示信息",
            std::bind(&StdinHandler::info_order, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
    }

    StdinHandler::~StdinHandler()
    {
    }

    void StdinHandler::input(int fd)
    {
        g_orders = &orderitems_;
        rl_callback_read_char();
    }

    void StdinHandler::quit_order(const OrderItem& order, int argc, char** argv)
    {
        rl_callback_handler_remove();
        struct timeval delay = { 2, 0 };
        LOG_INFO << "捕获到一个退出命令,程序将在2秒后安全退出";
        base()->loopexit(&delay);
    }

    void StdinHandler::info_order(const OrderItem& order, int argc, char** argv)
    {
        char path[PATH_MAX] = { 0 };
        char processname[NAME_MAX] = { 0 };
        shynet::utils::stuff::executable_path(path, processname, sizeof(path));
        LOG_INFO_BASE << "程序名:" << processname;
        LOG_INFO_BASE << "使用线程数:" << shynet::utils::stuff::num_of_threads();
        LOG_INFO_BASE << "进程id:" << getpid();
        shynet::utils::stuff::mem_info mem;
        shynet::utils::stuff::obtain_mem_info(&mem);
        LOG_INFO_BASE << "虚拟内存:" << mem.virt_kbytes << "kb";
        LOG_INFO_BASE << "常驻内存:" << mem.res_kbytes << "kb";
        LOG_INFO_BASE << "已运行时间:" << shynet::utils::stuff::up_duration_seconds() << "s";
    }
}
}
