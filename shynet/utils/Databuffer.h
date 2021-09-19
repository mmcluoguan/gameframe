#pragma once
#include <algorithm>
#include <cstring>
#include <memory>

namespace shynet
{
	namespace utils {
		template <typename DataType = uint8_t, typename LenType = std::size_t>
		class Databuffer {
		public:
			/**
		    * @param init_capacity   初始化大小，后续不够时内部会自动扩容，两倍增长
		    * @param shrink_capacity 收缩阀值，当申请内存大于<shrink_capacity>且实际使用小于<init_capacity>时会释发多余内存，
		    *                        恢复成<init_capacity>
		    *
		    */
			explicit Databuffer(LenType init_capacity = 16384, LenType shrink_capacity = 1048576) 
				: init_capacity_(init_capacity)
				, shrink_capacity_(shrink_capacity)
				, capacity_(init_capacity)
				, read_index_(0)
				, write_index_(0){
				data_ = new DataType[init_capacity];
			}

			/**
		    * shynet::utils::basic_buffer(data, len);
		    * 等价于
		    * shynet::utils::basic_buffer buf(len, 2 * len);
		    * buf.append(data, len);
		    *
		    * @param data 初始化数据，内部做内存拷贝
		    * @param len  初始化数据长度
		    *
		    */
			Databuffer(const DataType* data, LenType len)
				: init_capacity_(len)
				, shrink_capacity_(len * 2)
				, capacity_(len)
				, read_index_(0)
				, write_index_(0) {
				data_ = new DataType[capacity_];
				append(data, len);
			}

			/// 析构，释放内部内存
			~Databuffer() {
				delete[] data_;
				data_ = nullptr;
			}

			/// 拷贝构造以及赋值函数，内部执行深拷贝
			Databuffer(const Databuffer& b) : init_capacity_(b.init_capacity_)
				, shrink_capacity_(b.shrink_capacity_)
				, capacity_(b.capacity_)
				, read_index_(0)
				, write_index_(0) {
				data_ = new DataType[capacity_];
				append(b.read_pos(), b.readable_size());
			}
			Databuffer& operator=(const Databuffer& b) {
				if (this != &b) {
					read_index_ = 0;
					write_index_ = 0;
					append(b.read_pos(), b.readable_size());
				}
				return *this;
			}

			/**
			* 追加数据，空余空间不足时自动扩容
			*
			* @param data 追加数据，内部做内存拷贝
			* @param len  追加数据长度
			*
			*/
			void append(const DataType* data, LenType len) {
				reserve(len);
				memcpy(data_ + write_index_, data, len);
				write_index_ += len;
			}

			/*
			* 若内部空余空间小于<len>则会扩容新空间
			*/
			void reserve(LenType len) {
				if (capacity_ - write_index_ >= len) {
					return;
				}
				else if (capacity_ - write_index_ + read_index_ >= len) {
					memmove(data_, data_ + read_index_, write_index_ - read_index_);
				}
				else {
					LenType need_len = write_index_ - read_index_ + len;
					for (; capacity_ < need_len; capacity_ <<= 1);
					DataType* new_buf = new DataType[capacity_];
					memcpy(new_buf, data_ + read_index_, write_index_ - read_index_);
					delete[] data_;
					data_ = new_buf;
				}
				write_index_ -= read_index_;
				read_index_ = 0;
			}

			/*
			* 写位置
			*/
			DataType* write_pos() const {
				return data_ + write_index_;
			}

			/*
			* 往后挪动写位置
			*/
			void seek_write_pos(LenType len) {
				write_index_ += len;
			}

			// 将写位置往前移动，丢弃尾部部分数据
			void seek_write_pos_rollback(LenType len) {
				write_index_ -= len;
			}

			/**
			* 读取buffer中的数据
			*
			*/
			DataType* read_pos() const {
				return data_ + read_index_;
			}

			/*
			* 可读取大小
			*/
			LenType readable_size() const {
				return write_index_ - read_index_;
			}

			/*
			* 删除数据
			*/
			void erase(LenType len) {
				if (write_index_ - read_index_ >= len) {
					read_index_ += len;
					if (write_index_ - read_index_ < init_capacity_ &&
						capacity_ > shrink_capacity_)
					{
						DataType* new_data = new DataType[init_capacity_];
						memcpy(new_data, data_ + read_index_, write_index_ - read_index_);
						write_index_ -= read_index_;
						read_index_ = 0;
						delete[]data_;
						data_ = new_data;
						capacity_ = init_capacity_;
					}
				}
			}

			/**
			* 清空，注意：
			* 1. 并不会释放内部内存，只是将空间全部标记为空闲，内部申请的内存只有在析构时释放
			* 2. 如果capacity已经大于shrink阈值了，则收缩成init capacity大小
			*
			*/
			void clear() {
				read_index_ = write_index_ = 0;
				if (capacity_ > shrink_capacity_) {
					capacity_ = init_capacity_;
					delete[] data_;
					data_ = new DataType[capacity_];
				}
			}

			/// 已申请内存大小
			LenType capacity() const { 
				return capacity_; 
			}

			/**
			* @return 找到返回key位置，失败返回NULL
			*
			*/
			DataType* find(const DataType* key, LenType len) const {
				if (readable_size() == 0) {
					return nullptr;
				}
				DataType* pos = std::search(read_pos(), write_pos(), const_cast<DataType*>(key),
					const_cast<DataType*>(key) + len
				);
				return pos == data_ + write_index_ ? NULL : pos;
			}
			DataType* find(DataType c) const {
				if (readable_size() == 0) {
					return nullptr;
				}
				return static_cast<DataType*>(memchr(read_pos(), c, readable_size()));
			}
			DataType* find_crlf() const {
				return find((const DataType*)"\r\n", 2);
			}
			DataType* find_eol() const {
				return find('\n');
			}

			/**
			* 删除 '空格' '\f' '\r' '\n' '\t' '\v'
			*
			* @return 返回操作后的读取位置
			*
			*/
			DataType* trim_left() {
				for (; write_index_ != read_index_; ++read_index_) {
					DataType ch = *(data_ + read_index_);
					if (!std::isspace(ch)) {
						break;
					}
				}
				return read_pos();
			}
			DataType* trim_right() {
				for (; write_index_ != read_index_; --write_index_) {
					DataType ch = *(data_ + write_index_ - 1);
					if (!std::isspace(ch)) {
						break;
					}
				}
				return read_pos();
			}
		private:
			const LenType  init_capacity_;
			const LenType  shrink_capacity_;
			LenType        capacity_;
			LenType        read_index_ = 0;
			LenType        write_index_ = 0;
			DataType* data_;
		};
	}
}