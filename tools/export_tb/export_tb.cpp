#include "3rd/xl/libxl.h"
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

void export_lua(Sheet* sheet, const string& out_lua)
{
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
                    temp << sheet->readStr(row, col);
                } else {
                    temp << "\"" << sheet->readStr(row, col) << "\"";
                }
            }
            out << data_head[0][col] << " = " << temp.str() << ",";
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
        out << "[" << ids[i] << "] = items[" << i + 1 << "],";
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
    vector<string> ids;

    ostringstream out;
    out << "{" << endl;
    out << sheet->name() << ":{" << endl;
    for (int row = 3; row < sheet->lastRow(); ++row) {
        out << sheet->readStr(row, 0) << ":{" << endl;
        for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col) {
            string type = data_head[2][col];
            CellType cellType = sheet->cellType(row, col);
            ostringstream temp;
            if (cellType == CELLTYPE_EMPTY) {
                temp << "";
            } else if (cellType == CELLTYPE_NUMBER) {
                if (type == "int")
                    temp << (int)sheet->readNum(row, col);
                else
                    temp << sheet->readNum(row, col);
            } else if (cellType == CELLTYPE_STRING) {
                if (type == "luatable") {
                    temp << "";
                    //temp << sheet->readStr(row, col);
                } else {
                    temp << "\"" << sheet->readStr(row, col) << "\"";
                }
            }
            out << "{" << endl
                << data_head[0][col] << " : " << temp.str() << ",";
            if (data_head[0][col] == "Id")
                ids.push_back(temp.str());
        }
        out << "},";
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
            if (!out_lua.empty())
                export_lua(sheet, out_lua);
            if (!out_json.empty())
                export_json(sheet, out_json);
        } else {
            cout << "表头数据不足:" << xlsname;
        }

    } else {
        cout << "不能解析:" << xlsname;
    }
    book->release();
}
