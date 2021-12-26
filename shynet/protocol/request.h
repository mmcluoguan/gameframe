#ifndef SHYNET_PROTOCOL_REQUEST_H
#define SHYNET_PROTOCOL_REQUEST_H

#include "shynet/basic.h"
#include <map>

namespace shynet {
namespace protocol {
    /**
     * @brief http��������
    */
    class Request : public Nocopy {
    public:
        /**
         * @brief ����
        */
        Request() = default;
        ~Request() = default;

        /**
         * @brief δ��ʼ�׶�
         * @param line ����
         * @return 0�ɹ�,-1ʧ��
        */
        int requset_uninit(const std::string& line);
        /**
         * @brief ��ʼ�׶�
         * @param line ����
         * @return 0�ɹ�,-1ʧ��
        */
        int requset_init(const std::string& line);

        /**
         * @brief ��ȡЭ�鴦����׶�
         * @return Э�鴦����׶�
        */
        Step step() const { return step_; }
        /**
         * @brief ����Э�鴦����׶�
         * @param v Э�鴦����׶�
        */
        void set_step(Step v) { step_ = v; }

        /**
         * @brief ��ȡpost���������ݳ���
         * @return post���������ݳ���
        */
        size_t data_length() const;

        /**
         * @brief ��ȡwebsocket��key
         * @return websocket��key
        */
        const char* websocket_key() const;

        /**
         * @brief ��ȡ�ͻ�������ʽ get|post
         * @return �ͻ�������ʽ get|post
        */
        std::string method() const { return method_; }

        /**
         * @brief ��ȡhttp�汾
         * @return http�汾
        */
        std::string version() const { return version_; }
        /**
         * @brief ��ȡ�ͻ��������url
         * @return �ͻ��������url
        */
        std::string url() const
        {
            return path_;
        }
        /**
         * @brief ��ȡ�ͻ���get�������
         * @return �ͻ���get�������
        */
        const std::map<std::string, std::string>& params() { return params_; }
        /**
         * @brief ��ȡ�ͻ�������httpͷ
         * @return �ͻ�������httpͷ
        */
        const std::map<std::string, std::string>& heads() { return heads_; }

    private:
        /**
         * @brief �ͻ�������ʽ get|post
        */
        std::string method_;
        /**
         * @brief http�汾
        */
        std::string version_;
        /**
         * @brief �ͻ��������url
        */
        std::string path_;
        /**
         * @brief �ͻ���get�������
        */
        std::map<std::string, std::string> params_;
        /**
         * @brief �ͻ�������httpͷ
        */
        std::map<std::string, std::string> heads_;
        /**
         * @brief Э�鴦����׶�
        */
        Step step_ = Step::UNINIT;
    };
}
}

#endif
