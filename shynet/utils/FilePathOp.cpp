#include "shynet/utils/FilePathOp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

namespace shynet {
	namespace utils {
		int FilePathOp::exist(const std::string& name)
		{
			if (name.empty()) 
				return -1;
			struct stat st;
			return stat(name.c_str(), &st);
		}
		int FilePathOp::is_dir(const std::string& pathname)
		{
			if (pathname.empty())
				return -1;
			struct stat st;
			if (stat(pathname.c_str(), &st) == -1) {
				return -1;
			}
			return S_ISDIR(st.st_mode) ? 0 : -1;
		}
		int FilePathOp::is_abs_path(const std::string& name)
		{
			return !name.empty() && (name[0] == '/');
		}
		int FilePathOp::mkdir_recursive(const std::string& pathname)
		{
			if (pathname.empty())
				return -1;
			char* path_dup = strdup(pathname.c_str());
			size_t len = strlen(path_dup);
			if (len == 0) {
				return -1;
			}
			size_t i = path_dup[0] == '/' ? 1 : 0;
			for (; i <= len; ++i) {
				if (path_dup[i] == '/' || path_dup[i] == '\0') {
					char ch = path_dup[i];
					path_dup[i] = '\0';
					if (::mkdir(path_dup, 0755) == -1 && errno != EEXIST) {
						free(path_dup);
						return -1;
					}
					path_dup[i] = ch;
				}
			}
			free(path_dup);
			return 0;
		}
		int FilePathOp::rm_file(const std::string& name)
		{
			if (name.empty())
				return -1;
			if (exist(name) == -1) {
				return 0;
			}
			if (is_dir(name) == 0) {
				return -1;
			}
			if (::unlink(name.c_str()) == -1) {
				return -1;
			}
			return 0;
		}
		int FilePathOp::rmdir_recursive(const std::string& pathname)
		{
			if (pathname.empty())
				return -1;
			if (exist(pathname) == -1) {
				return 0;
			}
			if (is_dir(pathname) == -1) {
				return -1;
			}

			DIR* open_ret = ::opendir(pathname.c_str());
			if (open_ret == nullptr)
				return -1;

			struct dirent entry;
			struct dirent* result = NULL;
			int ret = 0;
			for (;;) {
				if (::readdir_r(open_ret, &entry, &result) != 0) {
					break;
				}
				if (result == NULL) {
					break;
				}
				char* name = result->d_name;
				if (strcmp(name, ".") == 0 ||
					strcmp(name, "..") == 0
					) {
					continue;
				}
				std::string file_with_path = join(pathname, name);
				if (exist(file_with_path.c_str()) != 0) {
					continue;
				}
				if (is_dir(file_with_path.c_str()) == 0) {
					if (rmdir_recursive(file_with_path.c_str()) != 0) {
						ret = -1;
					}
				}
				else {
					if (rm_file(file_with_path.c_str()) != 0) {
						ret = -1;
					}
				}
			}

			if (open_ret) {
				::closedir(open_ret);
			}

			return (::rmdir(pathname.c_str()) == 0 && ret == 0) ? 0 : -1;
		}
		int FilePathOp::rename(const std::string& src, const std::string& dst)
		{
			if (src.empty() || dst.empty())
				return -1;
			return ::rename(src.c_str(), dst.c_str());
		}
		int FilePathOp::write_file(const std::string& filename, const std::string& content, bool append)
		{
			return write_file(filename, content.c_str(), content.length(), append);
		}
		int FilePathOp::write_file(const std::string& filename, const char* content, size_t content_size, bool append)
		{
			FILE* fp = fopen(filename.c_str(), append ? "ab" : "wb");
			if (fp == nullptr){
				return -1;
			}
			size_t written = fwrite(reinterpret_cast<const void*>(content), 1, content_size, fp);
			fclose(fp);
			return (written == content_size) ? 0 : -1;
		}
		int64_t FilePathOp::get_file_size(const std::string& filename)
		{
			if (filename.empty())
				return -1;
			if (exist(filename) == -1 || is_dir(filename) == 0) {
				return -1;
			}
			struct stat st;
			if (::stat(filename.c_str(), &st) == -1) {
				return -1;
			}
			return st.st_size;
		}
		std::unique_ptr<char[]> FilePathOp::read_file(const std::string& filename)
		{
			int64_t size = get_file_size(filename);
			if (size <= 0) {
				return nullptr;
			}
			return read_file(filename, size);
		}
		std::unique_ptr<char[]> FilePathOp::read_file(const std::string& filename, size_t content_size)
		{
			if (content_size == 0) {
				return nullptr;
			}
			std::unique_ptr<char[]> content(new char[content_size]);
			int64_t read_size = read_file(filename.c_str(), content.get(), content_size);
			if (read_size == -1) {
				content.release();
			}
			return content;
		}
		int64_t FilePathOp::read_file(const std::string& filename, char* content, size_t content_size)
		{
			FILE* fp = fopen(filename.c_str(), "rb");
			if (fp == nullptr) {
				return -1;
			}
			size_t read_size = fread(reinterpret_cast<void*>(content), 1, content_size, fp);
			fclose(fp);
			return read_size;
		}

		std::unique_ptr<char[]> FilePathOp::read_link(const std::string& filename, size_t content_size)
		{
			if (filename.empty() || content_size == 0) {
				return nullptr;
			}
			std::unique_ptr<char[]> content(new char[content_size]);
			ssize_t length = ::readlink(filename.c_str(), content.get(), content_size);
			if (length == -1) {
				content.release();
			}
			return content;
		}
		std::string FilePathOp::join(const std::string& path, const std::string& filename)
		{
			std::string ret;
			size_t path_length = path.length();
			size_t filename_length = filename.length();
			if (path_length == 0) {
				return filename;
			}
			if (filename_length == 0) {
				return path;
			}
			if (path[path_length - 1] == '/') {
				ret = path.substr(0, path_length - 1);
			}
			else {
				ret = path;
			}
			ret += "/";
			if (filename[0] == '/') {
				ret += filename.substr(1, filename_length - 1);
			}
			else {
				ret += filename;
			}
			return ret;
		}
	}
}
