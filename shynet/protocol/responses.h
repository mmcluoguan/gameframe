#ifndef SHYNET_PROTOCOL_RESPONSES_H
#define SHYNET_PROTOCOL_RESPONSES_H

#include "shynet/basic.h"
#include <map>

namespace shynet {
namespace protocol {
    /**
     * @brief http响应处理器
    */
    class Responses : public Nocopy {
    public:
        Responses() = default;
        ~Responses() = default;

        /**
         * @brief 未初始阶段
         * @param line 数据
         * @return 0成功,-1失败
        */
        int responses_uninit(const std::string& line);
        /**
         * @brief 初始阶段
         * @param line 数据
         * @return 0成功,-1失败
        */
        int responses_init(const std::string& line);

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
         * @brief 获取http版本
         * @return http版本
        */
        std::string version() const { return version_; }
        /**
         * @brief 获取服务器响应状态码
         * @return 服务器响应状态码
        */
        int status_code() const { return status_code_; }
        /**
         * @brief 获取服务器响应状态描述
         * @return 服务器响应状态描述
        */
        std::string desc() const { return desc_; }
        /**
         * @brief 获取服务器响应http头
         * @return 服务器响应http头
        */
        const std::map<std::string, std::string>& heads() { return heads_; }

    private:
        /**
         * @brief http版本
        */
        std::string version_;
        /**
         * @brief 服务器响应状态码
        */
        int status_code_ = 200;
        /**
         * @brief 服务器响应状态描述
        */
        std::string desc_;
        /**
         * @brief 服务器响应http头
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
