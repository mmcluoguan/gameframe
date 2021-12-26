#ifndef SHYNET_PROTOCOL_HTTP_H
#define SHYNET_PROTOCOL_HTTP_H

#include "shynet/events/streambuff.h"
#include "shynet/protocol/request.h"
#include "shynet/protocol/responses.h"

namespace shynet {
namespace protocol {
    /**
     * @brief Э�������
    */
    class FilterProces;

    /**
     * @brief httpЭ�鴦����
    */
    class Http : public Nocopy {
    public:
        /**
         * @brief ����
         * @param filter Э�������
        */
        explicit Http(FilterProces* filter);
        ~Http() = default;

        /**
         * @brief ����http������Э��
         * @return ����io�������ݴ�����
        */
        net::InputResult process();
        /**
         * @brief ��������http������
         * @param inputbuffer io����
         * @param restore �������ݰ�������ʱ�ָ�ԭʼio����
         * @return ����io�������ݴ�����
        */
        net::InputResult process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        /**
         * @brief ������Ӧhttp������
         * @param inputbuffer io����
         * @param restore �������ݰ�������ʱ�ָ�ԭʼio����
         * @return ����io�������ݴ�����
        */
        net::InputResult process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        /**
         * @brief ������Ӧhttp����
         * @param data ָ�����ݵ�ָ��
         * @param len ָ�����ݵ�ָ���С
         * @param server_name ����������
         * @param content_type mime����
         * @param server_heads httpͷ
         * @param status_code ״̬��
         * @param status_desc ״̬����
         * @param version http�汾
         * @return 0�ɹ�,-1ʧ��
        */
        int send_responses(const void* data, size_t len,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief ������Ӧhttp����
         * @param data ����
         * @param server_name ����������
         * @param content_type mime���� 
         * @param server_heads httpͷ 
         * @param status_code ״̬��
         * @param status_desc ״̬����
         * @param version http�汾
         * @return 0�ɹ�,-1ʧ��
        */
        int send_responses(const std::string& data,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief ��������http����
         * @param data ָ�����ݵ�ָ��
         * @param len ָ�����ݵ�ָ���С
         * @param host ��������
         * @param path url·��
         * @param version http�汾
         * @return 0�ɹ�,-1ʧ��
        */
        int send_requset(const void* data, size_t len,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief ��������http����
         * @param data ����
         * @param host ��������
         * @param path url·��
         * @param version http�汾
         * @return  0�ɹ�,-1ʧ��
        */
        int send_requset(const std::string& data,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;

    private:
    private:
        FilterProces* filter_;
        Request requset_;
        Responses responses_;
    };
}
}

#endif
