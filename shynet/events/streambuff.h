#ifndef SHYNET_EVENTS_STREAMBUFF_H
#define SHYNET_EVENTS_STREAMBUFF_H

#include "shynet/basic.h"
#include <event2/buffer.h>
#include <memory>

namespace shynet {
namespace events {
    /**
     * @brief io缓冲
    */
    class Streambuff final : public Nocopy {
    public:
        /**
         * @brief 构造
        */
        Streambuff();
        /**
         * @brief 构造
         * @param buffer 原生io缓冲
        */
        explicit Streambuff(evbuffer* buffer);
        ~Streambuff();

        /**
         * @brief 获取原生io缓冲
         * @return 原生io缓冲
        */
        evbuffer* buffer() const;

        /**
         * @brief 加锁
        对于单个操作,不需要调用lock()和unlock():
        如果Streambuff启用了锁,单个操作就已经是原子的.只有在需要多个操作连续执行,
        不让其线程介入的时候,才需要手动锁定Streambuff
        */
        void lock() const;
        /**
         * @brief 解锁
        */
        void unlock() const;

        /**
         * @brief io缓冲当前数据大小
         * @return 
        */
        size_t length() const;

        /**
         * @brief 修改缓冲区的最后一块,或者添加一个新的块,使得缓冲区足以容纳len字节,
        而不需要更多的内存分配.
         * @param len 扩张的大小
         * @return 
        */
        int expand(size_t len) const;

        /**
         * @brief "线性化"buf前面的 size字节,
        必要时将进行复制或者移动,以保证这些字节是连续的,占据相同的内存块.
        如果size 是负的,函数会线性化整个缓冲区.如果size 大于缓冲区中的字节数,函数返回NULL.
        否则,evbuffer_pullup()返回指向buf 中首字节的指针.
         * @param size 大小
         * @return 指向buf 中首字节的指针
        */
        unsigned char* pullup(ssize_t size) const;
        /**
         * @brief 添加data处的len字节到自己的末尾
         * @param data 指向存储数据的缓冲区的指针
         * @param len 长度
         * @return 0成功,-1失败
        */
        int add(const void* data, size_t len) const;
        /**
         * @brief 添加格式化的数据到自己的末尾
         * @param fmt 格式
         * @param  格式化参数
         * @return 返回成功添加的字节,-1失败
        */
        int addprintf(const char* fmt, ...) const;
        /**
         * @brief 将src中的所有数据移动到自己的末尾
         * @param src io缓冲
         * @return 0成功,-1失败
        */
        int addbuffer(const std::shared_ptr<Streambuff> src) const;
        /**
         * @brief 添加data处的len字节到自己的开头
         * @param data 指向存储数据的缓冲区的指针
         * @param len 长度
         * @return 0成功,-1失败
        */
        int prepend(const void* data, size_t len) const;
        /**
         * @brief 将src中的所有数据移动到自己的开头
         * @param src io缓冲
         * @return 0成功,-1失败
        */
        int prependbuffer(const std::shared_ptr<Streambuff> src) const;

        /**
         * @brief 将从开始处移除自己数据中的len字节到data处的内存中,自己数据将减少
         如果可用字节少于len,函数复制所有字节.失败时返回-1,否则返回移除了的字节数.
         * @param data 指向将存储数据的缓冲区的指针
         * @param len 需要移除自己数据中的大小
         * @return 失败时返回-1,否则返回移除了的字节数.
        */
        int remove(void* data, size_t len) const;
        /**
         * @brief 将从开始处移除自己数据中的len字节到dst末尾,自己数据将减少
        如果字节数小于len,所有字节被移动.
         * @param dst io缓冲
         * @param len 需要移除自己数据中的大小
         * @return 失败时返回-1,否则返回移除了的字节数.
        */
        int removebuffer(const std::shared_ptr<Streambuff> dst, size_t len) const;
        /**
         * @brief 从开始处移除自己数据中的len字节,自己数据将减少
         * @param len 需要移除自己数据中的大小
         * @return 0成功,-1失败
        */
        int drain(size_t len) const;

        /**
         * @brief 将从开始处复制自己数据中的len字节到data处的内存中,自己数据不变
         如果可用字节少于len,函数会复制所有字节.失败时返回-1,否则返回复制的字节数
         * @param data 指向将存储数据的缓冲区的指针
         * @param len 需要复制自己数据中的大小
         * @return 失败时返回-1,否则返回复制的字节数
        */
        ssize_t copyout(void* data, size_t len) const;
        /**
         * @brief 从自己的开始处取出一行,用一个新分配的空字符结束的字符串返回这一行.
        如果n_read_out不是NULL,则它被设置为返回的字符串的字节数.
        如果没有整行供读取,函数返回NULL.返回的字符串不包括行结束符.
        返回的char*,需要外部释放
         * @param n_read_out 如果n_read_out不是NULL,则它被设置为返回的字符串的字节数.
         * @param style 换行标识 EVBUFFER_EOL_ANY,EVBUFFER_EOL_CRLF,EVBUFFER_EOL_LF,EVBUFFER_EOL_NUL
         * @return 如果没有整行供读取,函数返回NULL.返回的字符串不包括行结束符.
        否则返回的char*,需要外部释放
        */
        char* readln(size_t* n_read_out, evbuffer_eol_style style) const;

        /**
         * @brief 在缓冲区中查找含有len个字符的字符串what.
        返回包含字符串位置,或者在没有找到字符串时包含-1的evbuffer_ptr结构体.
        如果提供了start参数,则从指定的位置开始搜索;否则,从开始处进行搜索.
         * @param what 查找的字符串
         * @param len 查找的字符串的长度
         * @param start 开始查找位置
         * @return 一个结构 evbuffer_ptr，其'pos'字段具有'start'后缓冲区中字符串的第一次出现的位置,
        如果未找到字符串，则结果字段为 -1
        */
        evbuffer_ptr search(const char* what, size_t len, const struct evbuffer_ptr* start) const;
        /**
         * @brief 在缓冲区start到end的范围中查找含有len个字符的字符串what.
        返回包含字符串位置,或者在没有找到字符串时包含-1的evbuffer_ptr结构体.
         * @param what 查找的字符串
         * @param len 查找的字符串的长度
         * @param start 开始查找位置
         * @param end 结束查找位置
         * @return 一个结构 evbuffer_ptr，其'pos'字段具有'start'后缓冲区中字符串的第一次出现的位置,
        如果未找到字符串，则结果字段为 -1
        */
        evbuffer_ptr search_range(const char* what, size_t len,
            const struct evbuffer_ptr* start,
            const struct evbuffer_ptr* end) const;
        /**
         * @brief 查找指定换行风格的行,但是不复制行,而是返回指向行结束符的evbuffer_ptr.
        如果eol_len_out 非空,则它被设置为EOL 字符串长度
         * @param start 开始查找位置
         * @param eol_len_out 换行标识长度
         * @param eol_style 换行标识 EVBUFFER_EOL_ANY,EVBUFFER_EOL_CRLF,EVBUFFER_EOL_LF,EVBUFFER_EOL_NUL
         * @return 一个结构 evbuffer_ptr，其'pos'字段具有'start'后缓冲区中字符串的第一次出现的位置,
        如果未找到字符串，则结果字段为 -1
        */
        evbuffer_ptr search_eol(struct evbuffer_ptr* start,
            size_t* eol_len_out,
            evbuffer_eol_style eol_style) const;
        /**
         * @brief 偏移buffer中的位置 pos.
			如果how 等于EVBUFFER_PTR_SET,指针被移动到缓冲区中的绝对位置position;
			如果等于EVBUFFER_PTR_ADD,则向前移动position字节.
			成功时函数返回0,失败时返回-1
         * @param pos 偏移buffer位置
         * @param position 向前移动position字节
         * @param how EVBUFFER_PTR_SET,EVBUFFER_PTR_ADD
         * @return 0成功,-1失败
        */
        int ptr_set(struct evbuffer_ptr* pos,
            size_t position,
            enum evbuffer_ptr_how how) const;

        /**
         * @brief 调用peek()的时候,通过vec_out 给定一个evbuffer_iovec 数组,数组的长度是n_vec.
        函数会让每个结构体包含指向evbuffer 内部内存块的指针(iov_base)和块中数据长度.
        如果 len小于0,peek()会试图填充所有evbuffer_iovec 结构体.
        否则, 函数会进行填充, 直到使用了所有结构体, 或者见到len 字节为止.
        如果函数可以给出所有请求的数据, 则返回实际使用的结构体个数;
        否则, 函数返回给出所有请求数据所需的结构体个数.
        如果 ptr为 NULL, 函数从缓冲区开始处进行搜索.否则, 从ptr 处开始搜索.
        注意修改evbuffer_iovec所指的数据会导致不确定的行为.
        如果任何函数修改了evbuffer, 则peek()返回的指针会失效
        如果在多个线程中使用evbuffer, 确保在调用peek()之前使用lock(),
        在使用完peek()给出的内容之后进行解锁.
         * @param len 要尝试查看的字节数。 如果 len 为负，将尽可能多地填充 vec_out.
         如果 len 为负,  并且没有提供 vec_out，返回 evbuffer_iovecs 的数量 ,这将需要获取缓冲区中的所有数据
         * @param start_at buffer位置
         * @param vec_out evbuffer_iovec 数组
         * @param n_vec 数组的长度
         * @return 实际peek()获得的数组的长度
        */
        int peek(ssize_t len, struct evbuffer_ptr* start_at, struct evbuffer_iovec* vec_out, int n_vec) const;
        /**
         * @brief evbuffer_reserve_space()函数给出evbuffer内部空间的指针.
        函数会扩展缓冲区以至少提供size 字节的空间.到扩展空间的指针,以及其长度,
        会存储在通过vec 传递的向量数组中,n_vec是数组的长度.
        n_vec的值必须至少是1.如果只提供一个向量, event会确保请求的所有连续空间都在单个扩展区中,
        但是这可能要求重新排列缓冲区, 或者浪费内存.为取得更好的性能,
        应该至少提供2个向量.函数返回提供请求的空间所需的向量数.
        写入到向量中的数据不会是缓冲区的一部分, 直到调用evbuffer_commit_space(),
        使得写入的数据进入缓冲区.如果需要提交少于请求的空间,
        可以减小任何 evbuffer_iovec结构体的 iov_len字段,
        也可以提供较少的向量.函数成功时返回0, 失败时返回 - 1.
        提示和警告:
        调用任何重新排列evbuffer或者向其添加数据的函数都将使从evbuffer_reserve_space()获取的指针失效.
        当前实现中, 不论用户提供多少个向量, evbuffer_reserve_space()从不使用多于两个.
        未来版本可能会改变这一点.
        如果在多个线程中使用evbuffer, 确保在调用evbuffer_reserve_space()之前使用evbuffer_lock()进行锁定,
        然后在提交后解除锁定
         * @param size 
         * @param vec 
         * @param n_vecs 
         * @return 
        */
        int reserve_space(ssize_t size, struct evbuffer_iovec* vec, int n_vecs) const;
        /**
         * @brief 
         * @param vec 
         * @param n_vecs 
         * @return 
        */
        int commit_space(struct evbuffer_iovec* vec, int n_vecs) const;

        /**
         * @brief 与使用负的howmuch 参数调用write_atmost()一样:
         函数会试图尽量清空buffer 的内容.
         * @param fd socket文件描述符
         * @return 实际写入的字节数,-1失败
        */
        int write(evutil_socket_t fd) const;
        /**
         * @brief 试图将buffer 前面至多howmuch 字节写入到套接字fd 中.成功时函数返回写入的字节数,
        失败时返回-1.跟evbuffer_read()一样,应该检查错误码,看是真的错误,还是仅仅指示非阻塞IO不能立即完成.
        如果为howmuch 给出负值,函数会试图写入buffer 的所有内容.
         * @param fd socket文件描述符
         * @param howmuch 写入长度
         * @return 实际写入的字节数,-1失败
        */
        int write_atmost(evutil_socket_t fd, ssize_t howmuch) const;
        /**
         * @brief 从套接字fd 读取至多howmuch 字节到buffer 末尾.
         成功时函数返回读取的字节数,0表示EOF,失败时返回-1.
         注意,错误码可能指示非阻塞操作不能立即成功,应该检查错误码EAGAIN
         如果howmuch 为负,read()试图猜测要读取多少数据.
         * @param fd socket文件描述符
         * @param howmuch 读取长度
         * @return 实际读取的字节数,-1失败
        */
        int read(evutil_socket_t fd, int howmuch) const;

        /**
         * @brief 通过引用向 evbuffer末尾添加一段数据.不会进行复制:
        evbuffer只会存储一个到data处的 datlen字节的指针.
        因此,在evbuffer 使用这个指针期间,必须保持指针是有效的.
        evbuffer会在不再需要这部分数据的时候调用用户提供的 cleanupfn函数,
        带有提供的data指针、datlen值和 extra指针参数.
        函数成功时返回0,失败时返回-1.
         * @param data 指向外部存储数据的缓冲区的指针
         * @param datlen 存储数据的缓冲区大小
         * @param cleanupfn evbuffer不再需要这部分数据的时候调用回调
         * @param extra 附件指针
         * @return 0成功,-1失败
        */
        int add_reference(const void* data, size_t datlen,
            evbuffer_ref_cleanup_cb cleanupfn,
            void* extra) const;

    private:
        /**
         * @brief 原生的io缓冲
        */
        evbuffer* buffer_ = nullptr;
        /**
         * @brief 析构时是否释放原生的io缓冲
        */
        bool delflag_ = true;
    };
}
}

#endif
