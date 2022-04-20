#ifndef TEST_CONSISTENT_HASH_H
#define TEST_CONSISTENT_HASH_H

#include "shynet/utils/consistent_hash.h"

struct node {
    std::string ip;

    friend std::ostream& operator<<(std::ostream& out, const node& n)
    {
        out << n.ip;
        return out;
    }
};

void statistic_perf(shynet::utils::consistent_hash<node>& consistent, int min, int max)
{
    std::map<std::string, int> statistic;
    for (int i = min; i < max; i++) {
        std::optional<node> v = consistent.selectnode(i);
        if (v) {
            auto it = statistic.find(v->ip);
            if (it == statistic.end()) {
                statistic[v->ip] = 1;
            } else {
                it->second++;
            }
        }
    }
    for (auto& it : statistic) {
        std::cout << it.first << " " << it.second << std::endl;
    }
}

void test_consistent_hash()
{

    shynet::utils::consistent_hash<node> consistent(500);
    node n1 { "192.168.0.10" };
    node n2 { "192.168.0.11" };
    node n3 { "192.168.0.12" };
    consistent.addnode(n1);
    consistent.addnode(n2);
    consistent.addnode(n3);
    statistic_perf(consistent, 0, 1000);
    consistent.delnode(n2);
    statistic_perf(consistent, 0, 1000);
}
#endif
