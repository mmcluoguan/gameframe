#pragma once

#include "WukongMemoryPool.hpp"

/* 盘古内存池
* 提供内存块大小：1-65535byte
* 动态收缩：不支持
*/

namespace hzw {
namespace detail {
    //盘古内存池
    template <bool SupportPolym, template <typename T> typename Alloc>
    class PanguMemoryPool final : public MemoryPool<SupportPolym, true, UINT16_MAX> {
        using my_base = MemoryPool<SupportPolym, true, UINT16_MAX>;

    public:
        void* allocate(size_t size)
        {
            size = align_size(size);
            return size <= my_base::MAX_SIZE ? _pool[get_index(size)].allocate(size) : list_chain::forward_allocate(size);
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
            return "Pg";
        }

    private:
        using cookie_type = uint16_t;
        using list_chain = _ListChain<SupportPolym, cookie_type>;
        enum { ALIGN_SIZE = list_chain::ALIGN_SIZE };

        //功能：deallocate辅助函数（支持多态）
        void _deallocate(void* p, size_t, std::true_type)
        {
            size_t size { list_chain::get_cookie(p) };
            size == MEMORYPOOL_FORWARD_FLAG ? list_chain::forward_deallocate(p) : _pool[get_index(size)].deallocate(p);
        }

        //功能：deallocate辅助函数（不支持多态）
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
        std::map<size_t, list_chain, std::less<size_t>, Alloc<std::pair<const size_t, list_chain>>> _pool;
    };

    template <bool SupportPolym, template <typename T> typename Alloc> //盘古内存池别名
    using Pg = PanguMemoryPool<SupportPolym, Alloc>;
}

using PgG = detail::GMM<detail::Pg<false, AllocWkG>>; //盘古（不支持多态、全局归属）
using PgT = detail::TMM<detail::Pg<false, AllocWkT>>; //盘古（不支持多态、线程归属）
using PgGP = detail::GMM<detail::Pg<true, AllocWkG>>; //盘古（支持多态、全局归属）
using PgTP = detail::TMM<detail::Pg<true, AllocWkT>>; //盘古（支持多态、线程归属）

//盘古分配器别名
template <typename T>
using AllocPgG = detail::Allocator<T, PgG>;
template <typename T>
using AllocPgT = detail::Allocator<T, PgT>;

template <typename T, typename... Args>
auto make_unique_pgg(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocPgG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_unique_pgt(Args&&... args)
{
    return detail::make_unique_using_alloc<T, AllocPgT>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_pgg(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocPgG>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto make_shared_pgt(Args&&... args)
{
    return detail::make_shared_using_alloc<T, AllocPgT>(std::forward<Args>(args)...);
}

#if _HAS_CXX17
//盘古MemoryResource
detail::MemoryResource<PgG>* MRPgG()
{
    return detail::get_memory_resource<PgG>();
}
detail::MemoryResource<PgT>* MRPgT()
{
    return detail::get_memory_resource<PgT>();
}
#endif

using UsePgG = detail::UseMemoryPool<PgG>;
using UsePgT = detail::UseMemoryPool<PgT>;
using UsePgGP = detail::UseMemoryPool<PgGP>;
using UsePgTP = detail::UseMemoryPool<PgTP>;
}
