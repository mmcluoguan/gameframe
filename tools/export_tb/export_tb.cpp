#include "3rd/kaguya/kaguya.hpp"
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
    out << "local items = " << endl;
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

    out << "local idItems = " << endl;
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
 * @brief 导出json文件
 * @param sheet 表结构
 * @param out_json 导出的json路径
 * @return 是否成功
*/
bool export_json(Sheet* sheet, const string& out_json)
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
    out << " " << sheet->name() << ":{" << endl;
    for (int row = 2; row < sheet->lastRow(); ++row) {
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
            out << " " << data_head[0][col] << ":" << temp.str();
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
    return true;
}

extern int optind, opterr, optopt;

int main(int argc, char* argv[])
{
    string xlsname; // = "cfg/demo_cfg.xls";
    string out_lua; // = "../lua/config/demo_cfg.lua";
    string out_json; // = "../lua/config/demo_cfg.json";
    const char* opts = ":x:l:j:";
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
        return 0;
    }

    Book* book = xlCreateBook();

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
                bool ret = export_json(sheet, out_json);
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
    return 0;
}
