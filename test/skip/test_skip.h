#ifndef TEST_SKIP_H
#define TEST_SKIP_H

#include "3rd/memory/MemoryPool.hpp"
#include "shynet/basic.h"
#include "shynet/utils/skiplist.h"
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

void test_skip()
{
    struct cbstruct {
        int operator()(const std::pair<const std::string, int>& a,
            const std::pair<const std::string, int>& b) const
        {
            //>升序 <降序
            if (a.second < b.second) {
                return 1;
            } else if (a.second == b.second) {
                if (a.first >= b.first) {
                    return 1;
                }
                return -1;
            } else {
                return -1;
            }
        }
    };

    auto cb = [](const std::pair<const std::string, int>& a,
                  const std::pair<const std::string, int>& b) {
        //>升序 <降序
        if (a.second < b.second) {
            return 1;
        } else if (a.second == b.second) {
            if (a.first >= b.first) {
                return 1;
            }
            return -1;
        } else {
            return -1;
        }
    };

    std::function<int(const std::pair<const std::string, int>&,
        const std::pair<const std::string, int>&)>
        cbfun = std::bind(cb, std::placeholders::_1, std::placeholders::_2);

#define USER_ALLOC hzw::AllocWkG
    //shynet::utils::SkipList<std::string, int, USER_ALLOC, decltype(cbfun)> sk(cbfun);
    //shynet::utils::SkipList<std::string, int, USER_ALLOC, decltype(cb)> sk(cb);
    //shynet::utils::SkipList<std::string, int, USER_ALLOC, cbstruct> sk(cbstruct {});
    shynet::utils::SkipList<std::string, int, USER_ALLOC> sk;
    sk.insert({ "a", 100 });
    sk.insert({ "b", 200 });
    sk.insert({ "c", 150 });
    sk.insert({ "d", 210 });
    sk.insert({ "f", 90 });
    sk.insert({ "g", 250 });
    sk.insert({ "h", 210 });

    std::cout << "===============" << std::endl;
    for (auto&& [key, socre] : sk) {
        std::cout << "(" << key << "," << socre << ") ";
    }
    std::cout << std::endl
              << "===============" << std::endl;

    std::cout << sk.debug_string();
    auto it = sk.update("h", 230);
    //std::cout << sk.debug_string();
    sk.update(it.first, 250);
    std::cout << sk.debug_string();
    std::cout << sk << std::endl;
    auto fit = sk.find("f");
    std::cout << "(" << fit->first << "," << fit->second << "," << sk.pos_rank(fit) << ") " << std::endl;
    sk.erase("d");
    std::cout << sk.debug_string();
    auto rank_pos = sk.rank_pos(5);
    if (rank_pos != sk.end())
        std::cout << "(" << rank_pos->first << "," << rank_pos->second << ") " << std::endl;
    /*shynet::utils::SkipList<std::string, int, USER_ALLOC> sk1(sk);
	std::cout << sk1.debug_string();
	shynet::utils::SkipList<std::string, int, USER_ALLOC> sk2(std::move(sk1));
	std::cout << sk1.debug_string();
	std::cout << sk2.debug_string();
	shynet::utils::SkipList<std::string, int, USER_ALLOC> sk3 = std::move(sk2);
	std::cout << sk2.debug_string();
	std::cout << sk3.debug_string();*/

    auto itff = sk.find(80, 200);
    for (; itff.first != itff.second; ++itff.first) {
        auto& pair = *itff.first;
        std::cout << "(" << pair.first << "," << pair.second << ") ";
    }
    std::cout << std::endl;

    auto rit = sk.rank_rang(2, 1000);
    for (; rit.first != rit.second; ++rit.first) {
        auto& pair = *rit.first;
        std::cout << "(" << pair.first << "," << pair.second << ") ";
    }

    std::cout << std::endl;
}
#endif
