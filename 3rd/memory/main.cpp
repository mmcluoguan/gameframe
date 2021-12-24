#include "MemoryPool.hpp"
#include <list>
#include <chrono>
#include <fstream>

using namespace std::chrono;
using namespace hzw;

//模块使用介绍
class ModuleUse
{
public:
	static void entry()
	{
#if(0)
		//核心模块使用（以悟空做为范例，其他内存管理方案类似）
		core_module();
#endif

#if(0)
		//调试模块使用（debug模式开启，release模式关闭）
		debug_module();
#endif

#if(0)
		//工具模块使用
		tool_module();
#endif
	}

private:
	//核心模块使用（以悟空做为范例，其他内存管理方案类似）
	static void core_module()
	{
		//WkG
		void* wkg{ WkG::allocate(10) };//从WkG分配10byte内存
		WkG::deallocate(wkg, 10);//释放10byte内存到WkG
		//WkT
		void* wkt{ WkT::allocate(10) };//从WkT分配10byte内存
		WkT::deallocate(wkt, 10);//释放10byte内存到WkT
		//WkGP
		void* wkgp{ WkGP::allocate(10) };
		WkGP::deallocate(wkgp);//无需指明大小
		//WkTP
		void* wktp{ WkTP::allocate(10) };
		WkTP::deallocate(wktp);//无需指明大小
	}

	//调试模块使用（debug模式下调试模块才生效）
	static void debug_module()
	{
#if(0)
		set_memorypool_log_handle(file_log);//设置调试模块输出位置为文件（默认为控制台）
#endif

		//调试信息使用（在内存泄漏和内存使用错误时显示）
		WkG::allocate(20, "内存泄漏");//此处代码内存泄漏将显示调试信息 "内存泄漏"
		for (size_t i{ 0 }; i < 2; ++i)
		{
			for (size_t j{ 0 }; j < 2; ++j)
				if (i == 1 && j == 1)
					WkG::allocate(20, "debug_module中", "循环 i=", i, " j=", j, " 内存泄漏");
		}//调试信息不限数量不限类型，只需信息类型重载<<运算符

		//预定义调试信息
		WkG::allocate(20, DI_LOCATION);//显示内存泄漏所在文件和所在行数
		WkG::allocate(20, DI_IGNORE);//此处内存泄漏不显示
	}

	//工具模块使用
	static void tool_module()
	{
		//Allocator搭配容器使用
		std::list<int, AllocWkG<int>> list;

		//UseMemoryPool搭配自定义对象使用
		class AClass : public UseWkG {};
		AClass* aClass{ new AClass };//从WkG申请内存
		delete aClass;//释放内存到WkG

#if _HAS_CXX17
	//MemoryResource搭配std::pmr下容器（c++17起有效）
		std::pmr::list<int> pmrList{ MRWkG() };
#endif

		struct AObject { AObject(int i, double d) {} };
		//使用内存管理方案构造对象，并返回unique_ptr智能指针
		auto uniqueObject{ make_unique_wkg<AObject>(1, 1.1) };
		//使用内存管理方案构造对象，并返回shared_ptr智能指针
		auto sharedObject{ make_shared_wkg<AObject>(1, 1.1) };
	}

	static std::ostream& file_log()
	{
		static std::ofstream ostream("memorypool_log.txt");
		return ostream;
	}
};

//性能基准测试
class Performance
{
public:
	static void entry()
	{
#if(0)
		//malloc/free基准测试
		test<std::allocator>();
#endif
#if(0)
		//悟空测试
		test<AllocWkT>();
#endif
#if(0)
		//洛基测试
		test<AllocLkT>();
#endif
#if(0)
		//盘古测试
		test<AllocPgT>();
#endif
#if(0)
		//女娲测试
		test<AllocNwT>();
#endif
	}

private:
	enum { CONTAIN_SIZE = 5'000'000, TEST_COUNT = 10 };

	struct A { char buf[1]; };
	struct B { char buf[32]; };
	struct C { char buf[64]; };
	struct D { char buf[90]; };

	template<template<typename T> typename Alloc>
	static void test()
	{
		/*开启4个线程，每个线程执行如下动作
		向list容器分别加入CONTAIN_SIZE的A、B、C、D对象，并全部释放
		反复上述操作TEST_COUNT次*/

		steady_clock::time_point t{ steady_clock::now() };

		std::thread t1{ test_imp<Alloc>, TEST_COUNT, CONTAIN_SIZE };
		std::thread t2{ test_imp<Alloc>, TEST_COUNT, CONTAIN_SIZE };
		std::thread t3{ test_imp<Alloc>, TEST_COUNT, CONTAIN_SIZE };
		std::thread t4{ test_imp<Alloc>, TEST_COUNT, CONTAIN_SIZE };

		t1.join();
		t2.join();
		t3.join();
		t4.join();

		std::cout << duration_cast<seconds>(steady_clock::now() - t).count();
	}

	template<template<typename T> typename Alloc>
	static void test_imp(int testCount, int containSize)
	{
		for (int i{ 0 }; i < testCount; ++i)
		{
			std::list<A, Alloc<A>> la;
			std::list<B, Alloc<B>> lb;
			std::list<C, Alloc<C>> lc;
			std::list<D, Alloc<D>> ld;
			for (int j = 0; j < containSize; ++j)
			{
				la.push_back(A{});
				lb.push_back(B{});
				lc.push_back(C{});
				ld.push_back(D{});
			}
		}
	}
};

//误用检测
class FoolproofDesign
{
public:
	static void entry()
	{
#if(0)
		//从其他地方获取的内存块（如malloc），却归还给本内存管理
		not_from_memorypool();
#endif

#if(0)
		//重复delete
		multi_delete();
#endif

#if(0)
		//内存越界
		overflow();
#endif

#if(0)
		//内存块获取的内存池和释放内存池不同
		diff_memorypool();
#endif

#if(0)
		//线程独占式的内存管理方案申请的内存，却在做为线程共享内存使用
		thread_error();
#endif

#if(0)
		//在不具有多态特性的内存管理方案中释放内存时，大小参数有误
		size_error();
#endif
	}

private:
	static void not_from_memorypool()
	{
		void* p{ malloc(10) };//从malloc处申请内存
		WkGP::deallocate(p);//在WkG出释放
	}

	static void multi_delete()
	{
		void* p{ WkGP::allocate(10, "multi_delete") };
		WkGP::deallocate(p);
		WkGP::deallocate(p);
	}

	static void overflow()
	{
		int* buf{ (int*)WkGP::allocate(sizeof(int) * 5, "overflow") };
		buf[5] = 10;//内存越界
	}

	static void diff_memorypool()
	{
		void* p{ WkG::allocate(10, "diff_memorypool") };//从悟空中获取
		PgGP::deallocate(p);//归还给盘古
	}

	static void thread_error()
	{
		int* res{ nullptr };
		std::thread getResThread{ [&res] {res = (int*)WkTP::allocate(sizeof(int), "thread_error"); } }; //在getResThread线程申请内存
		getResThread.join();
		WkTP::deallocate(res);//在主线程释放
	}

	static void size_error()
	{
		void* p{ WkG::allocate(10, "size_error") };
		WkG::deallocate(p, 20);
	}

};

int main(int argc, char* argv[])
{
#if(0)	
	/*模块使用介绍
	（1）介绍核心模块使用、调试模块使用、工具模块使用
	（2）entry为入口函数，开启entry中对应编译条件即可运行对应示例
	（3）调试模块使用  必须在debug模式下运行*/
	ModuleUse::entry();
#endif

#if(0)
	/*性能基准测试
	（1）切换至release模式 
	（2）entry为入口函数，开启entry中对应编译条件即可运行对应示例
	（3）默认设置下至少需要7G空闲内存以供测试，内存不足将异常终止
	（4）通过设置CONTAIN_SIZE可以调节测试所需内存*/
	Performance::entry();
#endif

#if(0)
	/*误用检测
	（1）切换至debug模式
	（2）entry为入口函数，开启entry中对应编译条件即可运行对应示例*/
	FoolproofDesign::entry();
#endif
}
