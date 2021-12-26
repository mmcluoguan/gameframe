#ifndef SHYNET_PROTOCOL_RESPONSES_H
#define SHYNET_PROTOCOL_RESPONSES_H

#include "shynet/basic.h"
#include <map>

namespace shynet {
namespace protocol {
    /**
     * @brief http��Ӧ������
    */
    class Responses : public Nocopy {
    public:
        Responses() = default;
        ~Responses() = default;

        /**
         * @brief δ��ʼ�׶�
         * @param line ����
         * @return 0�ɹ�,-1ʧ��
        */
        int responses_uninit(const std::string& line);
        /**
         * @brief ��ʼ�׶�
         * @param line ����
         * @return 0�ɹ�,-1ʧ��
        */
        int responses_init(const std::string& line);

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
         * @brief ��ȡhttp�汾
         * @return http�汾
        */
        std::string version() const { return version_; }
        /**
         * @brief ��ȡ��������Ӧ״̬��
         * @return ��������Ӧ״̬��
        */
        int status_code() const { return status_code_; }
        /**
         * @brief ��ȡ��������Ӧ״̬����
         * @return ��������Ӧ״̬����
        */
        std::string desc() const { return desc_; }
        /**
         * @brief ��ȡ��������Ӧhttpͷ
         * @return ��������Ӧhttpͷ
        */
        const std::map<std::string, std::string>& heads() { return heads_; }

    private:
        /**
         * @brief http�汾
        */
        std::string version_;
        /**
         * @brief ��������Ӧ״̬��
        */
        int status_code_ = 200;
        /**
         * @brief ��������Ӧ״̬����
        */
        std::string desc_;
        /**
         * @brief ��������Ӧhttpͷ
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
