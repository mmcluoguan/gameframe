#pragma once

#include "MemoryPoolUtility.hpp"

namespace hzw {
namespace detail {
    //内存池管理基类
    template <template <typename MemoryPool> typename MemoryPoolManage, typename MemoryPool, bool BelongGlobal>
    struct MemoryPoolManageBase {
        using memory_pool = MemoryPool;
        using memory_manage = MemoryPoolManage<memory_pool>;
        using belong_global = std::bool_constant<BelongGlobal>; //是否全局归属
        using belong_thread = std::bool_constant<!BelongGlobal>; //是否线程归属
        using support_polym = typename memory_pool::support_polym; //管理的内存是否支持多态
        using support_chunk = typename memory_pool::support_chunk; //内存分配是否支持大区块
        enum { MAX_SIZE = memory_pool::MAX_SIZE };

        MemoryPoolManageBase() = delete;

        //功能：分配内存资源
        //输入：内存请求数量，内存需求大小
        //输出：分配后的内存指针
        static void* callocate(size_t count, size_t size)
        {
            return memory_manage::allocate(count * size);
        }

        //功能：重新分配内存资源（内存池支持malloc_size才提供此功能）
        //输入：内存的指针，内存需求大小
        //输出：重分配后的内存指针
        static void* reallocate(void* p, size_t size)
        {
            size_t mallocSize { memory_manage::malloc_size(p) };
            void* newP { memory_manage::allocate(size) };
            memcpy(newP, p, size < mallocSize ? size : mallocSize);
            memory_manage::deallocate(p);
            return newP;
        }

        //功能：获取内存块大小（内存池支持多态才提供此功能）
        //输入：内存指针
        //输出：内存块大小
        static size_t malloc_size(void* p)
        {
            static_assert(support_polym::value, "memory pool must support polym");
            return memory_pool::malloc_size(p);
        }

        //功能：获取内存池名
        //输出：内存池名
        static const char* get_pool_name()
        {
            constexpr const char* poolName { MemoryPool::get_pool_name() };
            constexpr const char* belongName { belong_global::value ? "G" : "T" };
            constexpr const char* polymName { support_polym::value ? "P" : "" };
            constexpr static auto str { MAKE_NATIVE_STR(poolName, belongName, polymName) };
            return str.c_str();
        }
    };

    //全局内存池管理：全局共用一个内存池
    template <typename MemoryPool>
    class GlobalMemoryPoolManage final : public MemoryPoolManageBase<GlobalMemoryPoolManage, MemoryPool, true> {
        using my_base = MemoryPoolManageBase<GlobalMemoryPoolManage, MemoryPool, true>;

    public:
        //功能：分配内存资源
        //输入：内存需求大小
        //输出：分配后的内存指针
        template <typename... DebugInfos>
        static void* allocate(size_t size, DebugInfos&&...)
        {
            std::lock_guard<std::mutex> locker { _poolMutex() };
            return _memoryPool().allocate(size);
        }

        //功能：释放内存资源（不支持多态）
        //输入：释放内存的指针，释放内存大小
        static void deallocate(void* p, size_t size)
        {
            static_assert(!my_base::support_polym::value, "memory pool must unsupport polym");
            std::lock_guard<std::mutex> locker { _poolMutex() };
            _memoryPool().deallocate(p, size);
        }

        //功能：释放内存资源（支持多态）
        //输入：释放内存的指针
        static void deallocate(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            constexpr size_t unuse { 1 };
            std::lock_guard<std::mutex> locker { _poolMutex() };
            _memoryPool().deallocate(p, unuse);
        }

    private:
        static std::mutex& _poolMutex()
        {
            static std::mutex _poolMutex; //内存池互斥锁
            return _poolMutex;
        }

        static MemoryPool& _memoryPool()
        {
            static MemoryPool _memoryPool; //内存池
            return _memoryPool;
        }
    };

    //线程内存池管理：一个线程对应一个内存池
    template <typename MemoryPool>
    class ThreadLocalMemoryPoolManage final : public MemoryPoolManageBase<ThreadLocalMemoryPoolManage, MemoryPool, false> {
        friend class MemoryPoolProxy;
        using my_base = MemoryPoolManageBase<ThreadLocalMemoryPoolManage, MemoryPool, false>;

    public:
        //功能：分配内存资源
        //输入：内存需求的大小
        //输出：分配后的内存指针
        template <typename... DebugInfos>
        static void* allocate(size_t size, DebugInfos&&...)
        {
            return _memoryPoolProxy().allocate(size);
        }

        //功能：释放内存资源（不支持多态）
        //输入：释放内存的指针，释放内存的大小
        static void deallocate(void* p, size_t size)
        {
            static_assert(!my_base::support_polym::value, "memory pool must unsupport polym");
            _memoryPoolProxy().deallocate(p, size);
        }

        //功能：释放内存资源（支持多态）
        //输入：释放内存的指针
        static void deallocate(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            constexpr size_t unuse { 1 };
            _memoryPoolProxy().deallocate(p, unuse);
        }

    private:
        //内存池代理：提供内存池回收重利用功能
        class MemoryPoolProxy {
        public:
            MemoryPoolProxy()
                : _memoryPool { ini_pool() }
            {
            }

            MemoryPoolProxy(const MemoryPoolProxy&) = delete;

            MemoryPoolProxy(MemoryPoolProxy&&) = delete;

            MemoryPoolProxy& operator=(const MemoryPoolProxy&) = delete;

            MemoryPoolProxy& operator=(MemoryPoolProxy&&) = delete;

            ~MemoryPoolProxy() noexcept
            {
                std::lock_guard<std::mutex> locker { _poolsMutex() };
                _pools().push_back(std::move(_memoryPool));
            }

            void* allocate(size_t size)
            {
                return _memoryPool.allocate(size);
            }

            void deallocate(void* p, size_t size)
            {
                _memoryPool.deallocate(p, size);
            }

        private:
            static MemoryPool ini_pool()
            {
                std::lock_guard<std::mutex> locker { _poolsMutex() };
                if (_pools().empty())
                    return MemoryPool {};
                else {
                    MemoryPool pool { std::move(_pools().back()) };
                    _pools().pop_back();
                    return pool;
                }
            }

        private:
            MemoryPool _memoryPool;
        };

    private:
        static Vector<MemoryPool>& _pools()
        {
            static Vector<MemoryPool> _pools; //内存池容器
            return _pools;
        }

        static std::mutex& _poolsMutex()
        {
            static std::mutex _poolsMutex; //内存池容器锁
            return _poolsMutex;
        }

        static MemoryPoolProxy& _memoryPoolProxy()
        {
            static thread_local MemoryPoolProxy _memoryPoolProxy; //各线程独立的内存池代理
            return _memoryPoolProxy;
        }
    };

    //内存池管理外覆器
    template <typename MemoryPoolManage>
    struct MemoryPoolManageWrapBase {
        using memory_manage = MemoryPoolManage;
        using memory_pool = typename memory_manage::memory_pool;
        using belong_global = typename memory_manage::belong_global; //是否全局归属
        using belong_thread = typename memory_manage::belong_thread; //是否线程归属
        using support_polym = typename memory_manage::support_polym; //管理的内存是否支持多态
        using support_chunk = typename memory_manage::support_chunk; //内存分配是否支持大区块
        enum { MAX_SIZE = memory_manage::MAX_SIZE };

        MemoryPoolManageWrapBase() = delete;

        //功能：分配内存资源
        //输入：内存请求数量，内存需求大小
        //输出：分配后的内存指针
        static void* callocate(size_t count, size_t size)
        {
            return MemoryPoolManage::callocate(count, size);
        }

        //功能：重新分配内存资源（内存池支持malloc_size才提供此功能）
        //输入：内存的指针，内存需求大小
        //输出：重分配后的内存指针
        static void* reallocate(void* p, size_t size)
        {
            return MemoryPoolManage::reallocate(p, size);
        }

        //功能：获取内存块大小（内存池支持多态才提供此功能）
        //输入：内存指针
        //输出：内存块大小
        static size_t malloc_size(void* p)
        {
            return MemoryPoolManage::malloc_size(p);
        }

        //功能：获取内存池名
        //输出：内存池名
        static const char* get_pool_name()
        {
            return MemoryPoolManage::get_pool_name();
        }
    };

    //内存池管理外覆器（为其添加缓冲功能）
    template <typename MemoryPoolManage, bool Enable>
    class BufferMemoryPoolManageWrap final : public MemoryPoolManageWrapBase<MemoryPoolManage> {
        using my_base = MemoryPoolManageWrapBase<MemoryPoolManage>;

    public:
        using buffer_enable = std::bool_constant<Enable>; //缓冲是否生效

        //功能：分配内存资源
        //输入：内存需求大小
        //输出：分配后的内存指针
        template <typename... DebugInfos>
        static void* allocate(size_t size, DebugInfos&&...)
        {
            return _allocate(size, buffer_enable {});
        }

        //功能：释放内存资源（不支持多态）
        //输入：释放内存的指针，释放内存大小
        static void deallocate(void* p, size_t size)
        {
            static_assert(!my_base::support_polym::value, "memory pool must unsupport polym");
            _deallocate(p, size, buffer_enable {});
        }

        //功能：释放内存资源（支持多态）
        //输入：释放内存的指针
        static void deallocate(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            constexpr size_t unuse { 0 };
            _deallocate(p, unuse, buffer_enable {});
        }

    private:
        static void* _allocate(size_t size, std::true_type)
        {
            return get_buf().allocate(size);
        }

        static void* _allocate(size_t size, std::false_type)
        {
            return MemoryPoolManage::allocate(size);
        }

        static void _deallocate(void* p, size_t size, std::true_type)
        {
            get_buf().deallocate(p, size);
        }

        static void _deallocate(void* p, size_t size, std::false_type)
        {
            dealloc(p, size, typename my_base::support_polym {});
        }

        static void dealloc(void* p, size_t, std::true_type)
        {
            MemoryPoolManage::deallocate(p);
        }

        static void dealloc(void* p, size_t size, std::false_type)
        {
            MemoryPoolManage::deallocate(p, size);
        }

    private:
        class Buf {
        public:
            Buf()
                : _buf {}
                , _continuousDeallocCount { 0 }
                , _continuousAllocCount { 0 }
            {
            }

            Buf(const Buf&) = delete;

            Buf(Buf&&) = delete;

            Buf& operator=(const Buf&) = delete;

            Buf& operator=(Buf&&) = delete;

            ~Buf() = default; //进程结束系统回收

            void* allocate(size_t size)
            {
                ++_continuousAllocCount;
                _continuousDeallocCount = 0;
                if (_continuousAllocCount >= CONTINUOUS_COUNT_LIMIT)
                    return MemoryPoolManage::allocate(size);
                return _allocate(size, typename my_base::support_polym {});
            }

            void deallocate(void* p, size_t size)
            {
                _deallocate(p, size, typename my_base::support_polym {});
            }

        private:
            enum {
                BUF_SIZE = BUFFER_MEMORYPOOL_MANAGE_BUFFER_SIZE,
                CONTINUOUS_COUNT_LIMIT = BUF_SIZE / 10
            };

            void* _allocate(size_t size, std::true_type)
            {
                auto it { _buf.lower_bound(size) };
                if (it == std::end(_buf))
                    return MemoryPoolManage::allocate(size);
                if (it->first - size > (MEMORYPOOL_ALIGN_SIZE ? MEMORYPOOL_ALIGN_SIZE : 4))
                    return MemoryPoolManage::allocate(size);
                auto& vec { it->second };
                if (vec.empty())
                    return MemoryPoolManage::allocate(size);

                void* res { vec.back() };
                vec.pop_back();
                return res;
            }

            void* _allocate(size_t size, std::false_type)
            {
                auto it { _buf.find(size) };
                if (it == std::end(_buf))
                    return MemoryPoolManage::allocate(size);
                auto& vec { it->second };
                if (vec.empty())
                    return MemoryPoolManage::allocate(size);

                void* res { vec.back() };
                vec.pop_back();
                return res;
            }

            void _deallocate(void* p, size_t, std::true_type)
            {
                _deallocate(p, my_base::malloc_size(p), std::false_type {});
            }

            void _deallocate(void* p, size_t size, std::false_type)
            {
                //连续释放超过限制值则直接由内存池回收，不再进行缓冲
                _continuousAllocCount = 0;
                ++_continuousDeallocCount;
                if (_continuousDeallocCount >= CONTINUOUS_COUNT_LIMIT)
                    return dealloc(p, size, typename my_base::support_polym {});

                auto it { _buf.find(size) };
                if (it == std::end(_buf)) {
                    std::vector<void*> v;
                    v.reserve(BUF_SIZE);
                    v.emplace_back(p);
                    _buf.emplace(size, std::move(v));
                    return;
                }

                auto& vec { it->second };
                if (vec.size() == BUF_SIZE) {
                    for (auto p : vec)
                        dealloc(p, size, typename my_base::support_polym {});
                    vec.clear();
                }
                vec.emplace_back(p);
            }

        private:
            std::map<size_t, std::vector<void*>> _buf;
            size_t _continuousDeallocCount, _continuousAllocCount;
        };

    private:
        static Buf& get_buf()
        {
            static thread_local Buf buf;
            return buf;
        }
    };

    //内存池管理外覆器（为其添加内存泄漏检测、误用检测，仅debug模式下启用）
    template <typename MemoryPoolManage>
    class DebugMemoryPoolManageWrap final : public MemoryPoolManageWrapBase<MemoryPoolManage> {
        struct DebugCookie;
        using my_base = MemoryPoolManageWrapBase<MemoryPoolManage>;

    public:
        //功能：分配内存资源
        //输入：内存需求的大小
        //输出：分配后的内存指针
        template <typename... DebugInfos>
        static void* allocate(size_t sz, DebugInfos&&... debugInfos)
        {
            assert(sz > 0);
            //内存需求：原始需求 + 两个哨兵 + DebugCookie
            DebugCookie* cookie { static_cast<DebugCookie*>(MemoryPoolManage::allocate(sz + sizeof(guard_type) * 2 + sizeof(DebugCookie))) };
            std::lock_guard<std::mutex> locker { _mutex() }; //链表操作多线程必须保持一致性
            //记录内存池命中率
            ++(_head()._count);
            if (sz <= my_base::MAX_SIZE)
                ++(_head()._hitCount);
            const char* poolName { my_base::get_pool_name() };
            //构造DebugCookie
            new (cookie) DebugCookie { nullptr, _head()._end, create_debug_info(std::forward<DebugInfos>(debugInfos)...),
                sz, std::hash<std::thread::id> {}(std::this_thread::get_id()),
                poolName, MEMORYPOOL_FLAG };
            //链接双向链表
            if (_head()._begin == nullptr)
                _head()._begin = cookie;
            else
                _head()._end->_next = cookie;
            _head()._end = cookie;
            //设置哨兵
            guard_type* guardBegin { reinterpret_cast<guard_type*>(cookie + 1) };
            guard_type* guardEnd { reinterpret_cast<guard_type*>(reinterpret_cast<char*>(guardBegin + 1) + sz) };
            *guardEnd = *guardBegin = GUARD_FLAG;
            return guardBegin + 1;
        }

        //功能：释放内存资源（不支持多态）
        //输入：释放内存的指针，释放内存的大小
        static void deallocate(void* p, size_t sz) //不支持多态
        {
            static_assert(!my_base::support_polym::value, "memory pool must unsupport polym");
            assert(p != nullptr);
            DebugCookie* cookie { get_debug_cookie(p) };
            _deallocate(cookie, sz);
            MemoryPoolManage::deallocate(cookie, sz + sizeof(DebugCookie) + sizeof(guard_type) * 2);
        }

        //功能：释放内存资源（支持多态）
        //输入：释放内存的指针
        static void deallocate(void* p) //支持多态
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            assert(p != nullptr);
            constexpr size_t unuse { 0 };
            DebugCookie* cookie { get_debug_cookie(p) };
            _deallocate(cookie, unuse);
            MemoryPoolManage::deallocate(cookie);
        }

        //功能：分配内存资源
        //输入：内存请求数量，内存需求大小
        //输出：分配后的内存指针
        static void* callocate(size_t count, size_t size)
        {
            return allocate(count * size);
        }

        //功能：重新分配内存资源（内存池支持malloc_size才提供此功能）
        //输入：内存的指针，内存需求大小
        //输出：重分配后的内存指针
        static void* reallocate(void* p, size_t size)
        {
            size_t mallocSize { malloc_size(p) };
            void* newP { allocate(size) };
            memcpy(newP, p, size < mallocSize ? size : mallocSize);
            deallocate(p);
            return newP;
        }

        //功能：获取内存块大小（内存池支持多态才提供此功能）
        //输入：内存指针
        //输出：内存块大小
        static size_t malloc_size(void* p)
        {
            static_assert(my_base::support_polym::value, "memory pool must support polym");
            assert(p != nullptr);
            constexpr size_t unuse { 0 };
            DebugCookie* cookie { get_debug_cookie(p) };
            check_common(cookie, unuse);
            return MemoryPoolManage::malloc_size(cookie) - sizeof(DebugCookie) - sizeof(guard_type) * 2;
        }

    private:
        template <typename... DebugInfos>
        static std::string create_debug_info(DebugInfos&&... debugInfos)
        {
            std::ostringstream ostream;
            ostream << "[";
            _create_debug_info(ostream, std::forward<DebugInfos>(debugInfos)...);
            ostream << "]";
            return ostream.str();
        }

        static std::string create_debug_info()
        {
            return DI_NULL;
        }

        template <typename DebugInfo, typename... DebugInfos>
        static void _create_debug_info(std::ostringstream& os, DebugInfo&& debugInfo, DebugInfos&&... debugInfos)
        {
            os << std::forward<DebugInfo>(debugInfo);
            _create_debug_info(os, std::forward<DebugInfos>(debugInfos)...);
        }

        template <typename DebugInfo>
        static void _create_debug_info(std::ostringstream& os, DebugInfo&& debugInfo)
        {
            os << std::forward<DebugInfo>(debugInfo);
        }

        static DebugCookie* get_debug_cookie(void* p)
        {
            return reinterpret_cast<DebugCookie*>(static_cast<char*>(p) - sizeof(guard_type) - sizeof(DebugCookie));
        }

        static void check_common(DebugCookie* cookie, size_t size)
        {
            //校验是否从内存池申请
            if (cookie->_poolFlag != MEMORYPOOL_FLAG) {
                _memorypoolLogHandle() << "ERROR:" << cookie << " is not from hzw::MemoryPool or multiple delete\n";
                std::exit(1);
            }
            guard_type* guardBegin { reinterpret_cast<guard_type*>(cookie + 1) };
            guard_type* guardEnd { reinterpret_cast<guard_type*>(reinterpret_cast<char*>(guardBegin + 1) + cookie->_size) };
            //校验重复deallocate
            if (*guardEnd == DEALLOCATED_FLAG && *guardBegin == DEALLOCATED_FLAG) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << "\nERROR:multiple delete\n";
                std::exit(1);
            }
            //校验哨兵标记
            if (*guardBegin != GUARD_FLAG) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << "\nERROR:underflow\n";
                std::exit(1);
            }
            if (*guardEnd != GUARD_FLAG) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << "\nERROR:overflow\n";
                std::exit(1);
            }
            //校验是否从同名内存池分配释放
            if (cookie->_poolName != my_base::get_pool_name()) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << " \nERROR:allocate MemoryPool is different from dellocate MemoryPool\nFROM:"
                                       << cookie->_poolName << "\nNO FROM:" << my_base::get_pool_name() << std::endl;
                std::exit(1);
            }
            //校验是否在同一线程分配释放
            check_thread(cookie, typename my_base::belong_thread {}); //线程归属内存池才校验
            //校验释放大小
            check_size(cookie, size, typename my_base::support_polym {}); //非多态下校验
        }

        static void _deallocate(DebugCookie* cookie, size_t size)
        {
            check_common(cookie, size);
            //设置回收标记
            guard_type* guardBegin { reinterpret_cast<guard_type*>(cookie + 1) };
            guard_type* guardEnd { reinterpret_cast<guard_type*>(reinterpret_cast<char*>(guardBegin + 1) + cookie->_size) };
            *guardBegin = *guardEnd = DEALLOCATED_FLAG;
            //从双向链表移除
            std::lock_guard<std::mutex> locker { _mutex() }; //链表操作多线程必须保持一致性
            if (cookie->_prev == nullptr && cookie->_next == nullptr)
                _head()._begin = _head()._end = nullptr;
            else if (cookie->_prev == nullptr) {
                _head()._begin = cookie->_next;
                _head()._begin->_prev = nullptr;
            } else if (cookie->_next == nullptr) {
                cookie->_prev->_next = nullptr;
                _head()._end = cookie->_prev;
            } else {
                cookie->_prev->_next = cookie->_next;
                cookie->_next->_prev = cookie->_prev;
            }
        }

        //功能：校验释放大小（非多态下校验）
        static void check_size(DebugCookie* cookie, size_t size, std::false_type)
        {
            if (cookie->_size != size) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << "\nERROR:size is " << cookie->_size << " not " << size << "\n";
                std::exit(1);
            }
        }

        static void check_size(DebugCookie*, size_t, std::true_type) { }

        //功能：校验是否在同一线程分配释放（线程归属下校验）
        static void check_thread(DebugCookie* cookie, std::true_type)
        {
            if (cookie->_threadId != std::hash<std::thread::id> {}(std::this_thread::get_id())) {
                _memorypoolLogHandle() << "DEBUG_INFO:" << cookie->_debugInfo << "\nERROR:"
                                       << "allocate thread is different from dellocate thread\n";
                std::exit(1);
            }
        }

        static void check_thread(DebugCookie*, std::false_type) { }

    private:
        using guard_type = size_t;
        enum { GUARD_FLAG = SIZE_MAX,
            DEALLOCATED_FLAG = GUARD_FLAG - 1 };

        struct DebugCookie {
            DebugCookie* _next;
            DebugCookie* _prev;
            std::string _debugInfo; //内存调试消息
            size_t _size; //记录申请大小（不包含DebugCookie本身）
            size_t _threadId; //线程id
            const char* _poolName; //内存池名
            const char* _poolFlag; //内存池标记
        };

        struct DebugCookieHead {
            DebugCookieHead()
                : _begin { nullptr }
                , _end { nullptr }
                , _count { 0 }
                , _hitCount { 0 }
            {
                _memorypoolLogHandle(); //确保logHandle先于DebugCookieHead构造（先构造后析构）
            }

            ~DebugCookieHead()
            {
                _memorypoolLogHandle() << "\n----[" << my_base::get_pool_name() << "]  MEMORY_LEAK----\n";
                size_t size { 0 }, count { 0 };
                for (DebugCookie* head { _begin }; head; head = head->_next) {
                    if (head->_debugInfo == DI_IGNORE_CHECK)
                        continue;
                    size += head->_size;
                    ++count;
                    _memorypoolLogHandle() << "--------SIZE:" << head->_size << "  DEBUG_INFO:" << head->_debugInfo << std::endl;
                }
                _memorypoolLogHandle() << "----ALL_LEAK_SIZE:" << size << "  ALL_LEAK_COUNT:" << count << "----\n"
                                       << "----SERVICE RANGE:1-" << my_base::MAX_SIZE << "  HIT_RATE:" << static_cast<size_t>(static_cast<double>(_hitCount) / _count * 100) << "%----\n\n";
            }

            DebugCookie *_begin, *_end; //双向链表头尾节点
            size_t _count, _hitCount; //总内存请求次数、在服务大小的次数
        };

    private:
        static DebugCookieHead& _head()
        {
            static DebugCookieHead _head;
            return _head;
        }

        static std::mutex& _mutex()
        {
            static std::mutex _mutex;
            return _mutex;
        }
    };

#ifdef _DEBUG
    template <typename MemoryPool>
    using GMM = DebugMemoryPoolManageWrap<GlobalMemoryPoolManage<MemoryPool>>; //全局内存池管理别名（带debug功能）

    template <typename MemoryPool>
    using TMM = DebugMemoryPoolManageWrap<ThreadLocalMemoryPoolManage<MemoryPool>>; //线程内存池管理别名（带debug功能）
#else
    template <typename MemoryPool>
    using GMM = BufferMemoryPoolManageWrap<GlobalMemoryPoolManage<MemoryPool>, BUFFER_MANAGE_ENABLE>; //全局内存池管理别名

    template <typename MemoryPool>
    using TMM = ThreadLocalMemoryPoolManage<MemoryPool>; //线程内存池管理别名
#endif // _DEBUG
}
}
