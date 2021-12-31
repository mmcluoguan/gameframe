#include "3rd/kaguya/kaguya.hpp"
#include "3rd/rapidjson/document.h"
#include "3rd/xl/libxl.h"
#include "shynet/utils/stringop.h"
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>

using namespace libxl;
using namespace std;
using namespace rapidjson;
using namespace shynet::utils;

/**
 * @brief 表头
*/
vector<vector<string>> data_head;

/**
 * @brief 初始化表头
 * @param sheet 表解构
 * @return 是否成功
*/
bool init_datahead(Sheet* sheet)
{
    if (sheet->lastRow() > 2) {
        for (int row = 0; row < 2; ++row) {
            vector<string> data_row;
            for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
                data_row.push_back(sheet->readStr(row, col));
            }
            data_head.push_back(data_row);
        }

        if (data_head[0][0] != "id") {
            cout << "第0行第0列字段值不是:id"
                 << " 当前配置是:" << data_head[0][0] << endl;
            return false;
        }
        return true;
    }
    cout << "表头格式错误,正确为0行定义字段名,1行定义描述";
    return false;
}

/**
 * @brief lua表转换为json
 * @param state lua栈
 * @param luascript 配置的lua脚本
 * @param datajosn 输出转换的json
 * @return 是否成功
*/
bool luatojoson(kaguya::State& state, const char* luascript, string& datajosn)
{
    if (string(luascript) == "{}") {
        datajosn = "[]";
        return true;
    }
    string str = shynet::utils::stringop::str_format("data=%s", luascript);
    bool ret = state(str);
    if (ret == false) {
        cout << "错误的lua格式:" << luascript << endl;
        return ret;
    }
    ret = state("datajosn = rapidjson.encode(data,{sort_keys=true})");
    if (ret == false) {
        cout << "josn encode 失败" << endl;
        return ret;
    }
    string data = state["datajosn"];
    datajosn = data;
    return true;
}

/**
 * @brief 导出lua文件
 * @param sheet 表结构
 * @param out_lua 导出的lua路径
 * @return 是否成功
*/
bool export_lua(Sheet* sheet, const string& out_lua)
{
    kaguya::State state;
    vector<string> ids;

    ostringstream out;
    out << "local items =" << endl;
    out << "{" << endl;
    for (int row = 2; row < sheet->lastRow(); ++row) {
        out << "\t{";
        for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
            CellType cellType = sheet->cellType(row, col);
            ostringstream temp;
            if (cellType == CELLTYPE_EMPTY
                || cellType == CELLTYPE_BLANK
                || cellType == CELLTYPE_ERROR) {
                temp << "nil";
            } else if (cellType == CELLTYPE_NUMBER
                || cellType == CELLTYPE_STRING
                || cellType == CELLTYPE_BOOLEAN) {
                const char* v = sheet->readStr(row, col);
                string luascript = shynet::utils::stringop::str_format("local data=%s", v);
                bool ret = state(luascript);
                if (ret == false) {
                    cout << "错误的lua格式:" << v << endl;
                    return ret;
                }
                temp << v;
            }
            out << " " << data_head[0][col] << "=" << temp.str() << ",";
            if (data_head[0][col] == "id")
                ids.push_back(temp.str());
        }
        out << "},";
        out << endl;
    }
    out << "}" << endl;
    out << endl;

    out << "local idItems =" << endl;
    out << "{";
    out << endl;
    for (size_t i = 0; i < ids.size(); i++) {
        out << "\t[" << ids[i] << "]=items[" << i + 1 << "],";
        out << endl;
    }
    out << "}" << endl;
    out << endl;

    const char* end = R"(
local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
)";

    out << end;

    ofstream ofs(out_lua);
    ofs << out.str();
    return true;
}

/**
 * @brief c++结构定义数组
*/
vector<string> struct_define;

string foreach_array(string& struct_name, Value& obj);
string foreach_object(string& struct_name,
    Value& obj,
    vector<string>& keys);

/**
 * @brief 生成c++对应的struct定义
 * @param struct_name 结构名称
 * @param v1 json对象
*/
void build_object_define(string& struct_name,
    Value& v1)
{
    stringstream ss_object;
    string struct_name_copy = struct_name;
    vector<string> keys;
    ss_object << "struct " << struct_name << " {" << endl;
    ss_object << foreach_object(struct_name, v1, keys);
    ss_object << "};" << endl;
    stringstream keystr;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i != keys.size() - 1) {
            keystr << keys[i] << ", ";
        } else {
            keystr << keys[i];
        }
    }
    ss_object << "REFLECTION(" << struct_name_copy << ", " << keystr.str() << ")" << endl;
    struct_define.push_back(ss_object.str());
}

/**
 * @brief 遍历jsong对象
 * @param struct_name 结构名称
 * @param obj json对象
 * @param keys 收集遍历的字段
 * @return 对应c++类型
*/
string foreach_object(string& struct_name,
    Value& obj,
    vector<string>& keys)
{
    stringstream ss_field;
    for (auto& v1 : obj.GetObject()) {
        if (v1.value.GetType() == rapidjson::kStringType) {
            ss_field << "\tstd::string " << v1.name.GetString() << ";" << endl;
        } else if (v1.value.GetType() == rapidjson::kNumberType) {
            if (v1.value.IsInt())
                ss_field << "\tint " << v1.name.GetString() << ";" << endl;
            else if (v1.value.IsDouble())
                ss_field << "\tdouble " << v1.name.GetString() << ";" << endl;
        } else if (v1.value.GetType() == rapidjson::kArrayType) {
            string struct_name_copy = struct_name;
            struct_name.append("_");
            struct_name.append(v1.name.GetString());

            ss_field << "\tstd::vector<"
                     << foreach_array(struct_name, v1.value)
                     << "> " << v1.name.GetString() << ";" << endl;
            struct_name = struct_name_copy;
        } else if (v1.value.GetType() == rapidjson::kObjectType) {
            string struct_name_copy = struct_name;
            struct_name.append("_");
            struct_name.append(v1.name.GetString());
            ss_field << "\t" << struct_name << " " << v1.name.GetString() << ";" << endl;

            build_object_define(struct_name, v1.value);
            struct_name = struct_name_copy;
        }
        keys.push_back(v1.name.GetString());
    }
    return ss_field.str();
}

/**
 * @brief 遍历json数组
 * @param struct_name 结构名称
 * @param obj json数组
 * @return 对应c++类型
*/
string foreach_array(string& struct_name, Value& obj)
{
    string ret_type = "int";
    for (auto& m : obj.GetArray()) {
        if (m.GetType() == rapidjson::kStringType) {
            ret_type = "std::string";
        } else if (m.GetType() == rapidjson::kNumberType) {
            if (m.IsInt())
                ret_type = "int";
            else if (m.IsDouble())
                ret_type = "double";
        } else if (m.GetType() == rapidjson::kObjectType) {
            build_object_define(struct_name, m);
            ret_type = struct_name;
        }
        break;
    }
    return ret_type;
}

/**
 * @brief 导出c++元hpp字符串
 * @param out_mate c++元hpp文件名
 * @param jsondata json数据
 * @return c++元hpp字符串
*/
string export_mate(const string& out_mate, const string& jsondata)
{
    string struct_name = filesystem::path(out_mate).stem();
    string struct_name_src = struct_name;
    string fieldname;
    stringstream ss;
    ss << "struct " << struct_name << " {" << endl;
    Document doc;
    Value& obj = doc.Parse(jsondata.c_str());
    for (auto& m : obj.GetObject()) { //data
        ss << "\tstd::unordered_map<int, ";
        for (auto& v : m.value.GetObject()) { //[10001]:{}
            fieldname = m.name.GetString();
            struct_name.append("_");
            struct_name.append(m.name.GetString());
            ss << struct_name << "> " << fieldname << ";" << endl;

            build_object_define(struct_name, v.value);
            break;
        }
        break;
    }
    ss << "};" << endl;
    ss << "REFLECTION(" << struct_name_src << ", " << fieldname << ")" << endl;
    struct_define.push_back(ss.str());

    stringstream out;
    out << "#ifndef MATECFG_" << stringop::toupper(struct_name_src) << "_H" << endl;
    out << "#define MATECFG_" << stringop::toupper(struct_name_src) << "_H" << endl;
    out << R"(

#include "3rd/iguana/json.hpp"
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace matecfg {
)" << endl;

    for (auto& def : struct_define) {
        out << def.c_str() << endl;
    }

    out << "}" << endl;
    out << "#endif" << endl;
    return out.str();
}

/**
 * @brief 导出json文件
 * @param sheet 表结构
 * @param out_json 导出的json路径
 * @param out_mate 导出的mate路径
 * @return 是否成功
*/
bool export_json(Sheet* sheet, const string& out_json, const string& out_mate)
{
    kaguya::State state;
    state("package.cpath=package.cpath ..\";../luabin/?.so\"");
    bool ret = state("rapidjson = require('rapidjson')");
    if (ret == false) {
        cout << "没有安装rapidjson库" << endl;
        return ret;
    }
    ostringstream out;
    out << "{" << endl;
    out << " \"" << sheet->name() << "\":{" << endl;
    for (int row = 2; row < sheet->lastRow(); ++row) {

        if (sheet->cellType(row, 0) != CELLTYPE_NUMBER) {
            cout << "id列的值必须是数字" << endl;
            return false;
        }
        out << "\t"
            << "\"" << sheet->readStr(row, 0) << "\":{";
        for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
            CellType cellType = sheet->cellType(row, col);
            ostringstream temp;
            if (cellType == CELLTYPE_EMPTY
                || cellType == CELLTYPE_BLANK
                || cellType == CELLTYPE_ERROR) {
                temp << "\"\"";
            } else if (cellType == CELLTYPE_NUMBER
                || cellType == CELLTYPE_STRING
                || cellType == CELLTYPE_BOOLEAN) {
                string v = sheet->readStr(row, col);
                if (v == "nil") {
                    temp << "\"\"";
                } else {
                    string datajosn;
                    bool ret = luatojoson(state, sheet->readStr(row, col), datajosn);
                    if (!ret)
                        return ret;
                    else
                        temp << datajosn;
                }
            }
            out << " \"" << data_head[0][col] << "\":" << temp.str();
            if (col != sheet->lastCol() - 1) {
                out << ",";
            }
        }
        out << "}";
        if (row != sheet->lastRow() - 1) {
            out << ",";
        }
        out << endl;
    }
    out << " }" << endl;
    out << "}" << endl;
    out << endl;

    ofstream ofs(out_json);
    ofs << out.str();
    ofs.close();

    if (!out_mate.empty()) {
        ofstream mate_ofs(out_mate);
        mate_ofs << export_mate(out_mate, out.str());
        mate_ofs.close();
    }
    return true;
}

extern int optind, opterr, optopt;

int main(int argc, char* argv[])
{
    string xlsname; //= "cfg/demo_cfg.xls";
    string out_lua; // = "../lua/config/demo_cfg.lua";
    string out_json; // = "../lua/config/demo_cfg.json";
    string out_mate; // = "../../3rd/matecfg/demo_cfg.hpp";
    const char* opts = ":x:l:j:c:";
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case 'x':
            xlsname = optarg;
            break;
        case 'l':
            out_lua = optarg;
            break;
        case 'j':
            out_json = optarg;
            break;
        case 'c':
            out_mate = optarg;
            break;
        case ':':
            cout << " (-" << (char)optopt << ") 丢失参数 " << endl;
            break;
        case '?':
            cout << " (-" << (char)optopt << ") 未知选项 " << endl;
            break;
        }
    }
    if (xlsname.empty()) {
        cout << "正确格式 " << opts << endl;
        cout << "\t-x 目标xls文件" << endl;
        cout << "\t-l 导出lua文件" << endl;
        cout << "\t-j 导出josn文件" << endl;
        cout << "\t-c 导出mate文件" << endl;
        return 0;
    }

    Book* book = xlCreateBook();
    filesystem::path path = xlsname;
    if (filesystem::exists(path)) {
        if (book->load(xlsname.c_str())) {
            Sheet* sheet = book->getSheet(0);
            if (init_datahead(sheet)) {
                if (!out_lua.empty()) {
                    cout << "开始导出:" << xlsname << " ==> " << out_lua << endl;
                    bool ret = export_lua(sheet, out_lua);
                    if (ret) {
                        cout << "导出成功:" << xlsname << " ==> " << out_lua << endl;
                    } else {
                        cout << "导出失败:" << xlsname << " ==> " << out_lua << endl;
                    }
                }
                if (!out_json.empty()) {
                    cout << "开始导出:" << xlsname << " ==> " << out_json << endl;
                    bool ret = export_json(sheet, out_json, out_mate);
                    if (ret) {
                        cout << "导出成功:" << xlsname << " ==> " << out_json << endl;
                    } else {
                        cout << "导出失败:" << xlsname << " ==> " << out_json << endl;
                    }
                }
            } else {
                cout << "表头设置错误:" << xlsname << endl;
            }
        } else {
            cout << "不能解析:" << xlsname << endl;
        }
        book->release();
    } else {
        cout << "文件不存在:" << path << endl;
    }
    return 0;
}
