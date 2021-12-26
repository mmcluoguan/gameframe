#ifndef SHYNET_PROTOCOL_REQUEST_H
#define SHYNET_PROTOCOL_REQUEST_H

#include "shynet/basic.h"
#include <map>

namespace shynet {
namespace protocol {
    /**
     * @brief http请求处理器
    */
    class Request : public Nocopy {
    public:
        /**
         * @brief 构造
        */
        Request() = default;
        ~Request() = default;

        /**
         * @brief 未初始阶段
         * @param line 数据
         * @return 0成功,-1失败
        */
        int requset_uninit(const std::string& line);
        /**
         * @brief 初始阶段
         * @param line 数据
         * @return 0成功,-1失败
        */
        int requset_init(const std::string& line);

        /**
         * @brief 获取协议处理步骤阶段
         * @return 协议处理步骤阶段
        */
        Step step() const { return step_; }
        /**
         * @brief 设置协议处理步骤阶段
         * @param v 协议处理步骤阶段
        */
        void set_step(Step v) { step_ = v; }

        /**
         * @brief 获取post包含的数据长度
         * @return post包含的数据长度
        */
        size_t data_length() const;

        /**
         * @brief 获取websocket的key
         * @return websocket的key
        */
        const char* websocket_key() const;

        /**
         * @brief 获取客户端请求方式 get|post
         * @return 客户端请求方式 get|post
        */
        std::string method() const { return method_; }

        /**
         * @brief 获取http版本
         * @return http版本
        */
        std::string version() const { return version_; }
        /**
         * @brief 获取客户端请求的url
         * @return 客户端请求的url
        */
        std::string url() const
        {
            return path_;
        }
        /**
         * @brief 获取客户端get请求参数
         * @return 客户端get请求参数
        */
        const std::map<std::string, std::string>& params() { return params_; }
        /**
         * @brief 获取客户端请求http头
         * @return 客户端请求http头
        */
        const std::map<std::string, std::string>& heads() { return heads_; }

    private:
        /**
         * @brief 客户端请求方式 get|post
        */
        std::string method_;
        /**
         * @brief http版本
        */
        std::string version_;
        /**
         * @brief 客户端请求的url
        */
        std::string path_;
        /**
         * @brief 客户端get请求参数
        */
        std::map<std::string, std::string> params_;
        /**
         * @brief 客户端请求http头
        */
        std::map<std::string, std::string> heads_;
        /**
         * @brief 协议处理步骤阶段
        */
        Step step_ = Step::UNINIT;
    };
}
}

#endif
