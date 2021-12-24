#pragma once

#include "MemoryPoolManage.hpp"

/* 悟空内存池
* 提供内存块大小：1-128byte
* 动态收缩：不支持
*/

namespace hzw {
namespace detail {
    //内存链（SupportPolym == false, CookieType无效）
    template <bool SupportPolym, typename CookieType>
    class _ListChain {
        enum { POINT_SIZE = sizeof(void*) };

    public:
        enum {
            ALIGN_SIZE = !MEMORYPOOL_ALIGN_SIZE ? POINT_SIZE : MEMORYPOOL_ALIGN_SIZE < POINT_SIZE ? POINT_SIZE
                                                                                                  : MEMORYPOOL_ALIGN_SIZE
        };

    public:
        _ListChain()
            : _size { 0 }
            , _head { nullptr }
            , _mallochead { nullptr }
        {
        }

        _ListChain(_ListChain&) = delete;

        _ListChain(_ListChain&&) = delete;

        _ListChain& operator=(_ListChain&) = delete;

        _ListChain& operator=(_ListChain&&) = delete;

        ~_ListChain()
        {
            if (_mallochead != nullptr) {
                FREE(_mallochead);
            }
        }

        void* allocate(size_t size)
        {
            if (_head == nullptr)
                fill_chain(size); //内存链为空则填充
            return remove_from_chain();
        }

        void deallocate(void* p)
        {
            add_to_chain(p);
        }

        //功能：读取cookie数值
        //template <typename = std::enable_if_t<SupportPolym>>
        static size_t get_cookie(void* p)
        {
            CookieType* cookie { reinterpret_cast<CookieType*>(static_cast<char*>(p) - COOKIE_SIZE) };
            return *cookie;
        }

        //功能：获取内存块大小
        //template <typename = std::enable_if_t<SupportPolym>>
        static size_t get_malloc_size(void* p)
        {
            return get_cookie(p) == MEMORYPOOL_FORWARD_FLAG ? MALLOC_SIZE(static_cast<char*>(p) - COOKIE_SIZE) - COOKIE_SIZE : get_cookie(p);
        }

        //功能：超出MAX_SIZE请求，调用MALLOC
        static void* forward_allocate(size_t size)
        {
            char* res { static_cast<char*>(MALLOC(size + COOKIE_SIZE)) };
            set_cookie(res, MEMORYPOOL_FORWARD_FLAG, support_polym {});
            return res + COOKIE_SIZE;
        }

        //功能：超出MAX_SIZE请求，调用FREE
        static void forward_deallocate(void* p)
        {
            FREE(static_cast<char*>(p) - COOKIE_SIZE);
        }

    private:
        struct Node;
        static_assert(MEMORYPOOL_ALIGN_SIZE == 0 || MEMORYPOOL_ALIGN_SIZE >= sizeof(CookieType), "cookie size must less than align size");

        using support_polym = std::bool_constant<SupportPolym>;
        enum {
            COOKIE_SIZE = SupportPolym ? (!MEMORYPOOL_ALIGN_SIZE ? sizeof(CookieType) : MEMORYPOOL_ALIGN_SIZE) : 0,
            MAX_SPLIT_SIZE = 128,
            MIN_SPLIT_SIZE = 20
        };

        //功能：设置cookie
        //输入：cookie地址，内存请求大小
        static void set_cookie(char* cookieP, size_t size, std::true_type)
        {
            CookieType* cookie { reinterpret_cast<CookieType*>(cookieP) };
            *cookie = static_cast<CookieType>(size);
        }

        static void set_cookie(char*, size_t, std::false_type) { }

        //功能：添加内存块到内存链
        //输入：添加的内存块
        void add_to_chain(void* p)
        {
            --_size;
            Node* np { static_cast<Node*>(p) };
            np->next = _head;
            _head = np;
        }

        //功能：从内存链移除内存块
        //输出：内存块
        void* remove_from_chain()
        {
            ++_size;
            void* result { _head };
            _head = _head->next;
            return result;
        }

        //功能：填充内存链
        //输入：内存需求大小
        void fill_chain(size_t size)
        {
            //对齐内存大小
            size = align_size(size);
            //动态决定切割个数
            size_t splitSize { MIN_SPLIT_SIZE + (_size >> 5) };
            splitSize = splitSize < MAX_SPLIT_SIZE ? splitSize : MAX_SPLIT_SIZE;
            //申请切割的内存
            char* chunk { static_cast<char*>(MALLOC(splitSize * size)) };
            //切割chunk
            char* p { chunk + COOKIE_SIZE };
            for (size_t i { 1 }; i < splitSize; ++i, p += size) {
                reinterpret_cast<Node*>(p)->next = reinterpret_cast<Node*>(p + size);
                set_cookie(p - COOKIE_SIZE, size - COOKIE_SIZE, support_polym {});
            }
            reinterpret_cast<Node*>(p)->next = nullptr;
            set_cookie(p - COOKIE_SIZE, size - COOKIE_SIZE, support_polym {});
            //挂到内存链上
            _head = reinterpret_cast<Node*>(chunk + COOKIE_SIZE);
            _mallochead = _head;
        }

        //功能：对齐内存请求大小
        static size_t align_size(size_t size)
        {
            return (COOKIE_SIZE + size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
        }

    private:
        struct Node {
            Node* next;
        };

        size_t _size; //当前内存链上内存块数
        Node* _head; //内存链头节点
        Node* _mallochead; //分配头
    };

    //悟空内存池
    template <bool SupportPolym>
    class WukongMemoryPool final : public MemoryPool<SupportPolym, false, 128> {
        using my_base = MemoryPool<SupportPolym, false, 128>;

    public:
        WukongMemoryPool()
            : _pool(CHAIN_LENTH)
        {
        }

        void* allocate(size_t size)
        {
            size = align_size(size);
            if (size <= my_base::MAX_SIZE)
                return _pool[get_index(size)].allocate(size);
            else
                return list_chain::forward_allocate(size);
        }

        void deallocate(void* p, size_t size)
        {
            _deallocate(p, size, typename my_base::support_polym {});
        }

        static size_t malloc_size(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            return list_chain::get_malloc_size(p);
        }

        constexpr static const char* get_pool_name()
        {
            return "Wk";
        }

    private:
        using cookie_type = uint8_t;
        using list_chain = _ListChain<SupportPolym, cookie_type>;
        enum { ALIGN_SIZE = list_chain::ALIGN_SIZE,
            CHAIN_LENTH = my_base::MAX_SIZE / ALIGN_SIZE };

        //功能：_deallocate辅助函数（支持多态）
        void _deallocate(void* p, size_t, std::true_type)
        {
            static_assert(my_base::support_polym::value, "true");
            size_t size { list_chain::get_cookie(p) };
            size == MEMORYPOOL_FORWARD_FLAG ? list_chain::forward_deallocate(p) : _pool[get_index(size)].deallocate(p);
        }

        //功能：_deallocate辅助函数（不支持多态）
        void _deallocate(void* p, size_t size, std::false_type)
        {
            size = align_size(size);
            size > my_base::MAX_SIZE ? list_chain::forward_deallocate(p) : _pool[get_index(size)].deallocate(p);
        }

        static size_t get_index(size_t size)
        {
            return size / ALIGN_SIZE - 1;
        }

        static size_t align_size(size_t size)
        {
            return (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
        }

    private:
        std::vector<list_chain> _pool; //内存池
    };

    template <bool SupportPolym> //悟空内存池别名
    using Wk = WukongMemoryPool<SupportPolym>;
}

using WkG = detail::GMM<detail::Wk<false>>; //悟空（不支持多态、全局归属）
using WkT = detail::TMM<detail::Wk<false>>; //悟空（不支持多态、线程归属）
using WkGP = detail::GMM<detail::Wk<true>>; //悟空（支持多态、全局归属）
using WkTP = detail::TMM<detail::Wk<true>>; //悟空（支持多态、线程归属）

//悟空分配器别名
template <typename T>
using AllocWkG = detail::Allocator<T, WkG>;
template <typename T>
using AllocWkT = detail::Allocator<T, WkT>;

template <typename T, typename... Args>
auto make_unique_wkg(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocWkG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_unique_wkt(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocWkT>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_wkg(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocWkG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_wkt(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocWkT>(std::forward<Args>(args)...);
}

#if _HAS_CXX17
//悟空MemoryResource
detail::MemoryResource<WkG>* MRWkG()
{
    return detail::get_memory_resource<WkG>();
}
detail::MemoryResource<WkT>* MRWkT()
{
    return detail::get_memory_resource<WkT>();
}
#endif

using UseWkG = detail::UseMemoryPool<WkG>;
using UseWkT = detail::UseMemoryPool<WkT>;
using UseWkGP = detail::UseMemoryPool<WkGP>;
using UseWkTP = detail::UseMemoryPool<WkTP>;
}
