#include "test/mmpv/test_mmpv.h"
#include "3rd/MMKV/Core/MMKV.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
using namespace mmkv;

string to_string(vector<string>&& arr, const char* sp = ", ")
{
    string str;
    for (const auto& element : arr) {
        str += element;
        str += sp;
    }
    if (!str.empty()) {
        str.erase(str.length() - strlen(sp));
    }
    return str;
}

vector<string> arrIntKeys;

void functionalTest(MMKV* mmkv, bool decodeOnly)
{
    if (!decodeOnly) {
        mmkv->set(arrIntKeys, "arrIntKeys");
    }

    cout << "arrIntKeys = " << mmkv->getVector("arrIntKeys", arrIntKeys) << endl;
    cout << "arrIntKeys = " << ::to_string(std::move(arrIntKeys)) << endl;

    return;

    if (!decodeOnly) {
        mmkv->set(true, "bool");
    }
    cout << "bool = " << mmkv->getBool("bool") << endl;

    if (!decodeOnly) {
        mmkv->set(1024, "int32");
    }
    cout << "int32 = " << mmkv->getInt32("int32") << endl;

    if (!decodeOnly) {
        mmkv->set(numeric_limits<uint32_t>::max(), "uint32");
    }
    cout << "uint32 = " << mmkv->getUInt32("uint32") << endl;

    if (!decodeOnly) {
        mmkv->set(numeric_limits<int64_t>::min(), "int64");
    }
    cout << "int64 = " << mmkv->getInt64("int64") << endl;

    if (!decodeOnly) {
        mmkv->set(numeric_limits<uint64_t>::max(), "uint64");
    }
    cout << "uint64 = " << mmkv->getUInt64("uint64") << endl;

    if (!decodeOnly) {
        mmkv->set(3.14f, "float");
    }
    cout << "float = " << mmkv->getFloat("float") << endl;

    if (!decodeOnly) {
        mmkv->set(numeric_limits<double>::max(), "double");
    }
    cout << "double = " << mmkv->getDouble("double") << endl;

    if (!decodeOnly) {
        mmkv->set("Hello, MMKV-示例 for POSIX", "string");
    }
    string result;
    mmkv->getString("string", result);
    cout << "string = " << result << endl;

    cout << "allKeys: " << ::to_string(mmkv->allKeys()) << endl;
    cout << "count = " << mmkv->count() << ", totalSize = " << mmkv->totalSize() << endl;
    cout << "containsKey[string]: " << mmkv->containsKey("string") << endl;

    mmkv->removeValueForKey("bool");
    cout << "bool: " << mmkv->getBool("bool") << endl;
    mmkv->removeValuesForKeys({ "int", "long" });

    mmkv->set("some string", "null string");
    result.erase();
    mmkv->getString("null string", result);
    cout << "string before set null: " << result << endl;
    mmkv->set((const char*)nullptr, "null string");
    //mmkv->set("", "null string");
    result.erase();
    mmkv->getString("null string", result);
    cout << "string after set null: " << result << ", containsKey:" << mmkv->containsKey("null string") << endl;

    //kv.sync();
    //kv.async();
    //kv.clearAll();
    mmkv->clearMemoryCache();
    cout << "allKeys: " << ::to_string(mmkv->allKeys()) << endl;
    cout << "isFileValid[" << mmkv->mmapID() + "]: " << MMKV::isFileValid(mmkv->mmapID()) << endl;
}

int test_mmpv()
{
    for (size_t index = 0; index < 1000; index++) {
        arrIntKeys.push_back("int-" + to_string(index));
    }

    std::string rootDir = "mmkv";
    MMKV::initializeMMKV(rootDir);
    auto mmkv = MMKV::defaultMMKV();
    functionalTest(mmkv, false);
    return 0;
}
