#pragma once

#include "MemoryPoolManage.hpp"

/* 洛基内存池
* 提供内存块大小：1-128byte
* 动态收缩：支持
*/

namespace hzw {
namespace detail {
    //内存块（SupportPolym == false, CookieType无效）
    template <bool SupportAddr, bool SupportPolym, typename CookieType>
    class _Chunk {
    public:
        enum { ALIGN_SIZE = !MEMORYPOOL_ALIGN_SIZE ? 1 : MEMORYPOOL_ALIGN_SIZE };
        using addr_type = uint8_t;

    public:
        //输入：内存块大小、内存块个数
        _Chunk(size_t size, size_t blockSize)
            : _index { 0 }
            , _count { static_cast<addr_type>(blockSize < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : (blockSize < MAX_BLOCK_SIZE ? blockSize : MAX_BLOCK_SIZE)) }
            , _blockSize { _count }
            , _buf { static_cast<addr_type*>(MALLOC((align_size(size) + HEAD_SIZE) * _blockSize)) }
        {
            size = align_size(size);
            char* p { reinterpret_cast<char*>(_buf) + HEAD_SIZE };
            for (addr_type i { 1 }; i <= _blockSize; ++i, p += (size + HEAD_SIZE)) {
                *reinterpret_cast<addr_type*>(p) = i;
                set_addr(get_addr_point(p), i - 1, std::bool_constant<SupportAddr> {});
                set_cookie(get_cookie_point(p), static_cast<CookieType>(size), std::bool_constant<SupportPolym> {});
            }
        }

        _Chunk(const _Chunk&) = delete;

        _Chunk(_Chunk&& rh) noexcept
            : _index { rh._index }
            , _count { rh._count }
            , _blockSize { rh._blockSize }
            , _buf { rh._buf }
        {
            rh._buf = nullptr;
        }

        _Chunk& operator=(const _Chunk&) = delete;

        _Chunk& operator=(_Chunk&&) = delete;

        ~_Chunk() noexcept
        {
            FREE(_buf);
        }

        void* allocate(size_t size)
        {
            assert(!is_full());
            char* result { reinterpret_cast<char*>(_buf) + (align_size(size) + HEAD_SIZE) * _index + HEAD_SIZE };
            _index = *reinterpret_cast<addr_type*>(result);
            --_count;
            return result;
        }

        void deallocate(void* p, size_t size)
        {
            size = get_size(p, size, std::bool_constant<SupportPolym> {});
            assert(is_contain(p, size));
            ++_count;
            *static_cast<addr_type*>(p) = _index;
            _index = (get_addr_point(p) - _buf) / (size + HEAD_SIZE);
        }

        //功能：获取内存块大小
        //template <typename = std::enable_if_t<SupportPolym>>
        static size_t get_malloc_size(void* p)
        {
            constexpr size_t unuse { 0 };
            size_t cookieValue { get_size(p, unuse, std::true_type {}) };
            return cookieValue == MEMORYPOOL_FORWARD_FLAG ? MALLOC_SIZE(static_cast<char*>(p) - COOKIE_SIZE) - COOKIE_SIZE : cookieValue;
        }

        //功能：超出MAX_SIZE请求，调用MALLOC
        static void* forward_allocate(size_t size)
        {
            char* res { static_cast<char*>(MALLOC(size + COOKIE_SIZE)) };
            set_cookie(reinterpret_cast<CookieType*>(res), MEMORYPOOL_FORWARD_FLAG, std::bool_constant<SupportPolym> {});
            return res + COOKIE_SIZE;
        }

        //功能：超出MAX_SIZE请求，调用FREE
        static void forward_deallocate(void* p)
        {
            FREE(static_cast<char*>(p) - COOKIE_SIZE);
        }

        //功能：读取cookie数值
        //template <typename = std::enable_if_t<SupportPolym>>
        static size_t get_cookie(void* p)
        {
            constexpr size_t unuse { 0 };
            return get_size(p, 0, std::bool_constant<SupportPolym> {});
        }

        //功能：内存块未被使用
        bool is_empty() const
        {
            return _count == _blockSize;
        }

        //功能：内存块全部使用
        bool is_full() const
        {
            return !_count;
        }

        //功能：判断指针是否属于本chunk
        bool is_contain(void* p, size_t size) const
        {
            ptrdiff_t distance { get_addr_point(p) - _buf };
            return 0 <= distance && distance <= ((ptrdiff_t)get_size(p, size, std::bool_constant<SupportPolym> {}) + HEAD_SIZE) * (_blockSize - 1);
        }

        void swap(_Chunk& rh)
        {
            if (&rh != this) {
                using std::swap;
                swap(_buf, rh._buf);
                swap(_index, rh._index);
                swap(_count, rh._count);
                swap(_blockSize, rh._blockSize);
            }
        }

        //功能：获取可用块计数
        addr_type unuse_count() const
        {
            return _count;
        }

        //功能：获取内存块地址
        void* get_buf_point() const
        {
            return reinterpret_cast<void*>(_buf);
        }

        //功能：通过addr获取_buf
        //template <typename = std::enable_if_t<SupportAddr>>
        static void* get_buf_point(void* p, size_t size)
        {
            size = get_size(p, size, std::bool_constant<SupportPolym> {});
            return reinterpret_cast<void*>((static_cast<char*>(p) - HEAD_SIZE) - (HEAD_SIZE + size) * (*get_addr_point(p)));
        }

    private:
        enum {
            MAX_BLOCK_SIZE = 128,
            MIN_BLOCK_SIZE = 10,
            ADDR_SIZE = SupportAddr ? sizeof(addr_type) : 0,
            COOKIE_SIZE = SupportPolym ? sizeof(CookieType) : 0,
            HEAD_SIZE = !MEMORYPOOL_ALIGN_SIZE ? (ADDR_SIZE + COOKIE_SIZE) : MEMORYPOOL_ALIGN_SIZE
        };
        static_assert(MEMORYPOOL_ALIGN_SIZE == 0 || HEAD_SIZE <= MEMORYPOOL_ALIGN_SIZE, "cookie size must less than align size");

    private:
        //功能：获取body大小（支持多态）
        static size_t get_size(void* p, size_t, std::true_type)
        {
            return *get_cookie_point(p);
        }

        //功能：获取body大小（不支持多态）
        static size_t get_size(void*, size_t size, std::false_type)
        {
            return align_size(size);
        }

        static addr_type* get_addr_point(void* p)
        {
            return reinterpret_cast<addr_type*>(static_cast<char*>(p) - HEAD_SIZE);
        }

        static CookieType* get_cookie_point(void* p)
        {
            return reinterpret_cast<CookieType*>(static_cast<char*>(p) - COOKIE_SIZE);
        }

        static void set_cookie(CookieType* cookie, CookieType value, std::true_type)
        {
            *cookie = value;
        }

        static void set_cookie(CookieType*, CookieType, std::false_type) { }

        static void set_addr(addr_type* addr, addr_type value, std::true_type)
        {
            *addr = value;
        }

        static void set_addr(addr_type*, addr_type, std::false_type) { }

        static size_t align_size(size_t size)
        {
            return (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
        }

    private:
        addr_type _index; //可用块索引
        addr_type _count; //可用块计数
        addr_type _blockSize; //内存块个数
        addr_type* _buf; //内存块
    };

    //内存块链
    template <bool SupportPolym, typename CookieType>
    class _ChunkChain {
        using Chunk = _Chunk<false, SupportPolym, CookieType>;

    public:
        enum { ALIGN_SIZE = Chunk::ALIGN_SIZE };

    public:
        _ChunkChain()
            : _chunks {}
            , _allocateIt {}
            , _deallocateIt {}
            , _allocateState { true }
        {
        }

        _ChunkChain(const _ChunkChain&) = delete;

        _ChunkChain(_ChunkChain&&) = default;

        _ChunkChain& operator=(const _ChunkChain&) = delete;

        _ChunkChain& operator=(_ChunkChain&&) = delete;

        ~_ChunkChain() = default;

        void* allocate(size_t size)
        {
            size_t chunksSize { _chunks.size() };
            if (!chunksSize)
                goto fill; //chunks为空则填充
            if (!_allocateState && _allocateIt->is_full()) //非连续分配状态和上次分配位置无空闲，则重新查找空闲块
            {
                /*
					查询空闲内存块
					从上一次分配处向头尾逼近查找
					利用局部性原理，适配位置极可能就在失配处附近
					*/
                auto toBegin { _allocateIt };
                auto toEnd { std::next(_allocateIt) };
                bool atBegin { false };
                while (!atBegin || toEnd != _chunks.end()) //没到头或没到尾继续查找
                {
                    if (!atBegin) {
                        if (toBegin != _chunks.begin()) //没到头部
                        {
                            if (!toBegin->is_full()) {
                                _allocateIt = toBegin;
                                break;
                            }
                            --toBegin;
                        } else //到达头部
                        {
                            atBegin = true;
                            if (!toBegin->is_full()) {
                                _allocateIt = toBegin;
                                break;
                            }
                        }
                    }
                    if (toEnd != _chunks.end()) {
                        if (!toEnd->is_full()) {
                            _allocateIt = toEnd;
                            break;
                        }
                        ++toEnd;
                    }
                }
            }
            //查无空闲内存块则填充
            if (_allocateIt->is_full()) {
                _allocateState = true;
            fill:
                size_t blockSize { chunksSize >> 1 }; //动态设定blockSize
                _chunks.emplace_back(size, blockSize);
                _allocateIt = std::prev(_chunks.end());
            }
            _deallocateIt = _allocateIt; //1.下次内存回收位置可能就是当前分配位置 2.迭代器可能失效
            return _allocateIt->allocate(size);
        }

        void deallocate(void* p, size_t size)
        {
            if (!_deallocateIt->is_contain(p, size)) //失配则重新查找
            {
                //查找对应的chunk（与allocate查找原理相同）
                auto toBegin { _deallocateIt };
                auto toEnd { _deallocateIt };
                bool atBegin { false };
                for (;;) {
                    if (!atBegin) {
                        if (toBegin != _chunks.begin()) {
                            if (toBegin->is_contain(p, size)) {
                                _deallocateIt = toBegin;
                                break;
                            }
                            --toBegin;
                        } else {
                            atBegin = true;
                            if (toBegin->is_contain(p, size)) {
                                _deallocateIt = toBegin;
                                break;
                            }
                        }
                    }
                    if (toEnd != _chunks.end()) {
                        if (toEnd->is_contain(p, size)) {
                            _deallocateIt = toEnd;
                            break;
                        }
                        ++toEnd;
                    }
                    assert(!atBegin || toEnd != _chunks.end()); //内存块不属于此内存池 或 delete指针与指向类型不匹配
                }
            }
            _allocateIt = _deallocateIt;
            _deallocateIt->deallocate(p, size);
            _allocateState = false;

            if (_deallocateIt->is_empty()) //chunk为空移动到chunks最后
            {
                if (_chunks.back().is_empty()) //有两个全空chunk，释放旧的一个
                {
                    if (_deallocateIt != std::prev(_chunks.end(), 1))
                        _chunks.pop_back();
                    if (_chunks.size() < (_chunks.capacity() >> 1)) {
                        typename ChunksIterator::difference_type allocateItIndex { _allocateIt - _chunks.begin() };
                        typename ChunksIterator::difference_type deallocateItIndex { _deallocateIt - _chunks.begin() };
                        _chunks.shrink_to_fit(); //收缩_chunks
                        //收缩可能导致原有迭代器失效，固重新赋值
                        _allocateIt = _chunks.begin() + allocateItIndex;
                        _deallocateIt = _chunks.begin() + deallocateItIndex;
                    }
                }
                _chunks.back().swap(*_deallocateIt);
            }
        }

        static void* forward_allocate(size_t size)
        {
            return Chunk::forward_allocate(size);
        }

        static void forward_deallocate(void* p)
        {
            return Chunk::forward_deallocate(p);
        }

        static size_t get_cookie(void* p)
        {
            return Chunk::get_cookie(p);
        }

        static size_t get_malloc_size(void* p)
        {
            return Chunk::get_malloc_size(p);
        }

    private:
        using ChunksIterator = typename std::vector<Chunk>::iterator;
        std::vector<Chunk> _chunks; //内存链
        ChunksIterator _allocateIt, _deallocateIt; //上一次分配、回收的位置
        bool _allocateState; //连续分配状态
    };

    //洛基内存池
    template <bool SupportPolym>
    class LokiMemoryPool final : public MemoryPool<SupportPolym, false, 128> {
        using my_base = MemoryPool<SupportPolym, false, 128>;

    public:
        LokiMemoryPool()
            : _bpool(CHAIN_SIZE)
            , _spool(3)
        {
        }

        void* allocate(size_t size)
        {
            size = align_size(size, default_align {});
            return size <= my_base::MAX_SIZE ? search_chain(size, default_align {}).allocate(size) : ChunkChain::forward_allocate(size);
        }

        void deallocate(void* p, size_t size)
        {
            _deallocate(p, size, typename my_base::support_polym {});
        }

        static size_t malloc_size(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            return ChunkChain::get_malloc_size(p);
        }

        constexpr static const char* get_pool_name()
        {
            return "Lk";
        }

    private:
        using cookie_type = uint8_t;
        using ChunkChain = _ChunkChain<SupportPolym, cookie_type>;
        using default_align = std::bool_constant<ChunkChain::ALIGN_SIZE == 1>;

        enum { ALIGN_SIZE = default_align::value ? 4 : ChunkChain::ALIGN_SIZE,
            CHAIN_SIZE = my_base::MAX_SIZE / ALIGN_SIZE };

        void _deallocate(void* p, size_t, std::true_type)
        {
            size_t size { ChunkChain::get_cookie(p) };
            size == MEMORYPOOL_FORWARD_FLAG ? ChunkChain::forward_deallocate(p) : search_chain(size, default_align {}).deallocate(p, size);
        }

        void _deallocate(void* p, size_t size, std::false_type)
        {
            size = align_size(size, default_align {});
            size > my_base::MAX_SIZE ? ChunkChain::forward_deallocate(p) : search_chain(size, default_align {}).deallocate(p, size);
        }

        //功能：查找对应chunks（默认对齐）
        ChunkChain& search_chain(size_t size, std::true_type)
        {
            return size <= 3 ? _spool[size - 1] : search_chain(size, std::false_type {});
        }

        //功能：查找对应chunks（其他对齐）
        ChunkChain& search_chain(size_t size, std::false_type)
        {
            return _bpool[align_size(size, default_align {}) / ALIGN_SIZE - 1];
        }

        static size_t align_size(size_t size, std::true_type)
        {
            return size <= 3 ? size : align_size(size, std::false_type {});
        }

        static size_t align_size(size_t size, std::false_type)
        {
            return (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
        }

    private:
        std::vector<ChunkChain> _bpool; //负责4-128byte
        std::vector<ChunkChain> _spool; //负责1、2、3byte
    };

    template <bool SupportPolym>
    using Lk = LokiMemoryPool<SupportPolym>; //洛基内存池别名
}

using LkG = detail::GMM<detail::Lk<false>>; //洛基（不支持多态、全局归属）
using LkT = detail::TMM<detail::Lk<false>>; //洛基（不支持多态、线程归属）
using LkGP = detail::GMM<detail::Lk<true>>; //洛基（支持多态、全局归属）
using LkTP = detail::TMM<detail::Lk<true>>; //洛基（支持多态、线程归属）

//洛基分配器别名
template <typename T>
using AllocLkG = detail::Allocator<T, LkG>;
template <typename T>
using AllocLkT = detail::Allocator<T, LkT>;

template <typename T, typename... Args>
auto make_unique_lkg(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocLkG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_unique_lkt(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocLkT>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_lkg(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocLkG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_lkt(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocLkT>(std::forward<Args>(args)...);
}

#if _HAS_CXX17
//洛基MemoryResource
detail::MemoryResource<LkG>* MRLkG()
{
    return detail::get_memory_resource<LkG>();
}

detail::MemoryResource<LkT>* MRLkT()
{
    return detail::get_memory_resource<LkT>();
}
#endif

//适配用户自定义类
using UseLkG = detail::UseMemoryPool<LkG>;
using UseLkT = detail::UseMemoryPool<LkT>;
using UseLkGP = detail::UseMemoryPool<LkGP>;
using UseLkTP = detail::UseMemoryPool<LkTP>;
}
