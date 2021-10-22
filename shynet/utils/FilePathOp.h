#ifndef SHYNET_UTILS_FILEPATHOP_H
#define SHYNET_UTILS_FILEPATHOP_H

#include "shynet/basic.h"
#include <memory>

namespace shynet {
namespace utils {
    class FilePathOp final : public Nocopy {
    public:
        // @return 0 存在，文件或文件夹都可    -1 不存在
        static int exist(const std::string& name);

        // @return 0 存在，且为文件夹    -1 不存在，或不是文件夹
        static int is_dir(const std::string& pathname);

        // @return 0 是绝对路径格式（以`/`字符开头）    -1 不是绝对路径
        static int is_abs_path(const std::string& name);

        // @return 0 创建成功，或创建前已经存在    -1 失败
        static int mkdir_recursive(const std::string& pathname);

        /**
			* @param name 文件名
			*
			* @return
			*    0 删除成功，或删除前就不存在
			*   -1 删除失败，或`name`是文件夹
			*
			*/
        static int rm_file(const std::string& name);

        /**
			* @param pathname 文件夹名
			*
			* @return
			*    0 删除成功，或删除前就不存在
			*   -1 删除失败，或`pathname`不是文件夹
			*
			*/
        static int rmdir_recursive(const std::string& pathname);

        /**
			* @param src 源文件
			* @param dst 目标文件
			*
			* @return 0 成功 -1 失败
			*
			*/
        static int rename(const std::string& src, const std::string& dst);

        /**
			* 写文件
			*
			* @param append 当文件已经存在时，true则追加至末尾，false则覆盖原文件内容
			* @return 0 成功 -1 失败
			*
			*/
        static int write_file(const std::string& filename, const std::string& content, bool append = false);

        /**
			 * 写文件
			 *
			 * @param append 当文件已经存在时，true则追加至末尾，false则覆盖原文件内容
			 * @return 0 成功 -1 失败
			 *
			 */
        static int write_file(const std::string& filename, const char* content, size_t content_size, bool append = false);

        /**
			* @注意
			* 对于/proc目录下的大多数文件，stat()不返回文件
			* st_size 字段中的大小； 相反，该字段返回值为 0。
			*
			* @return 文件大小，失败则返回-1
			*
			*/
        static int64_t get_file_size(const std::string& filename);

        /**
			* 读文件，对get_file_size()+read_file()的封装，更易于使用
			*
			* @return 成功返回文件内容，失败返回std::string()
			*
			*/
        static std::unique_ptr<char[]> read_file(const std::string& filename);

        /**
			 * 由于/proc下面的文件无法通过::stat()获取文件长度，所以提供参数让调用者填入一个fixed长度
			 *
			 */
        static std::unique_ptr<char[]> read_file(const std::string& filename, size_t content_size);

        /**
			 * @param filename     文件名
			 * @param content      传出参数，读取到的文件内容，内存由外部申请
			 * @param content_size 最大读入大小
			 *
			 * @return 成功返回实际读入大小，失败返回-1
			 *
			 */
        static int64_t read_file(const std::string& filename, char* content /*out*/, size_t content_size);

        /// @TODO 能否统一成一个接口，内部判断是否是否为link
        static std::unique_ptr<char[]> read_link(const std::string& filename, size_t content_size);

        /**
			 * @param path     目录
			 * @param filename 文件名
			 *
			 * 连接目录和文件名，解决`path`后面'/'和`filename`前面'/'是否存在，重复的问题
			 *
			*/
        static std::string join(const std::string& path, const std::string& filename);
    };
}
}

#endif
