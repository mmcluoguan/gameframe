#include "3rd/kaguya/kaguya.hpp"
#include "3rd/xl/libxl.h"
#include "shynet/utils/stringop.h"
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace libxl;
using namespace std;

vector<vector<string>> data_head;

bool init_datahead(Sheet* sheet)
{
    if (sheet->lastRow() > +3) {
        for (int row = 0; row < 3; ++row) {
            vector<string> data_row;
            for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
                data_row.push_back(sheet->readStr(row, col));
            }
            data_head.push_back(data_row);
        }
        return true;
    }
    return false;
}

bool luatojoson(kaguya::State& state, const char* luascript, string& datajosn)
{
    string str = shynet::utils::stringop::str_format("data=%s", luascript);
    bool ret = state(str);
    if (ret == false) {
        cout << "错误的lua格式:" << luascript << endl;
        return ret;
    }
    ret = state("datajosn = rapidjson.encode(data)");
    if (ret == false) {
        cout << "josn encode 失败" << endl;
        return ret;
    }
    string data = state["datajosn"];
    datajosn = data;
    return true;
}

void export_lua(Sheet* sheet, const string& out_lua)
{
    kaguya::State state;
    vector<string> ids;

    ostringstream out;
    out << "local items = " << endl;
    out << "{" << endl;
    for (int row = 3; row < sheet->lastRow(); ++row) {
        out << "    {";
        for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
            string type = data_head[2][col];
            CellType cellType = sheet->cellType(row, col);
            ostringstream temp;
            if (cellType == CELLTYPE_EMPTY) {
                temp << "nil";
            } else if (cellType == CELLTYPE_NUMBER) {
                if (type == "int")
                    temp << (int)sheet->readNum(row, col);
                else
                    temp << sheet->readNum(row, col);
            } else if (cellType == CELLTYPE_STRING) {
                if (type == "luatable") {
                    const char* v = sheet->readStr(row, col);
                    string luascript = shynet::utils::stringop::str_format("local data=%s", v);
                    bool ret = state(luascript);
                    if (ret == false) {
                        cout << "错误的lua格式:" << v << endl;
                        return;
                    }
                    temp << v;
                } else {
                    temp << "\"" << sheet->readStr(row, col) << "\"";
                }
            } else if (cellType == CELLTYPE_BLANK) {
                temp << "\"\"";
            }
            out << " " << data_head[0][col] << "=" << temp.str() << ",";
            if (data_head[0][col] == "Id")
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
        out << "[" << ids[i] << "]=items[" << i + 1 << "],";
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
}

void export_json(Sheet* sheet, const string& out_json)
{
    kaguya::State state;
    state("package.cpath=package.cpath ..\"../luabin/?.so\"");
    bool ret = state("rapidjson = require('rapidjson')");
    if (ret == false) {
        cout << "没有安装rapidjson库" << endl;
        return;
    }
    ostringstream out;
    out << "{" << endl;
    out << " " << sheet->name() << ":{" << endl;
    for (int row = 3; row < sheet->lastRow(); ++row) {
        out << "\t"
            << "\"" << sheet->readStr(row, 0) << "\":{";
        for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
            string type = data_head[2][col];
            CellType cellType = sheet->cellType(row, col);
            ostringstream temp;
            if (cellType == CELLTYPE_EMPTY) {
                temp << "\"\"";
            } else if (cellType == CELLTYPE_NUMBER) {
                if (type == "int")
                    temp << (int)sheet->readNum(row, col);
                else
                    temp << sheet->readNum(row, col);
            } else if (cellType == CELLTYPE_STRING) {
                if (type == "luatable") {
                    string v = sheet->readStr(row, col);
                    if (v == "nil") {
                        temp << "\"\"";
                    } else {
                        string datajosn;
                        bool ret = luatojoson(state, sheet->readStr(row, col), datajosn);
                        if (!ret)
                            return;
                        else
                            temp << datajosn;
                    }
                } else {
                    temp << "\"" << sheet->readStr(row, col) << "\"";
                }
            } else if (cellType == CELLTYPE_BLANK) {
                temp << "\"\"";
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
}

int main(int argc, char* argv[])
{
    Book* book = xlCreateBook();

    string xlsname = "cfg/task_cfg_new.xls";
    string out_lua = "cfg/task_cfg_new.lua";
    string out_json = "cfg/task_cfg_new.json";

    if (book->load(xlsname.c_str())) {
        Sheet* sheet = book->getSheet(0);
        if (init_datahead(sheet)) {
            if (!out_lua.empty()) {
                cout << "开始导出:" << xlsname << " ==> " << out_lua << endl;
                export_lua(sheet, out_lua);
                cout << "完成导出:" << xlsname << " ==> " << out_lua << endl;
            }
            if (!out_json.empty()) {
                cout << "开始导出:" << xlsname << " ==> " << out_json << endl;
                export_json(sheet, out_json);
                cout << "完成导出:" << xlsname << " ==> " << out_json << endl;
            }
        } else {
            cout << "表头数据不足:" << xlsname << endl;
        }
    } else {
        cout << "不能解析:" << xlsname << endl;
    }
    book->release();
    return 0;
}
