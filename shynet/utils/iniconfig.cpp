#include "shynet/utils/iniconfig.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stringop.h"
#include <fstream>

namespace shynet {
namespace utils {
    IniConfig::IniConfig(const char* filename)
    {
        inifilename_ = filename;
        std::ifstream reader;
        reader.open(filename, std::ios::in);
        if (reader.is_open() == false) {
            std::ostringstream err;
            err << "open:" << ((filename == nullptr) ? "null" : filename);
            THROW_EXCEPTION(err.str());
        } else {
            std::string section;
            nodes ns;

            while (!reader.eof()) {
                std::string::size_type lpos = 0;
                std::string::size_type rpos = 0;
                std::string::size_type epos = 0;
                std::string line, key, value;
                getline(reader, line);
                stringop::trim(line);
                if (line.empty() == false) {
                    lpos = line.find('[');
                    rpos = line.find(']');
                    if (line.npos != lpos && line.npos != rpos && rpos > lpos) {
                        if (section.empty() == false && ns.size() != 0) {
                            content_.insert({ section, ns });
                            ns.clear();
                        }
                        section = line.substr(lpos + 1, rpos - 1);
                        continue;
                    }
                    epos = line.find('=');
                    if (line.npos != epos) {
                        key = line.substr(0, epos);
                        value = line.substr(epos + 1, line.length() - 1);
                        stringop::trim(key);
                        stringop::trim(value);
                    }
                    if (section.empty() == false and key.empty() == false) {
                        ns.insert({ key, { section, key, value } });
                    }
                }
            }
            reader.close();
            if (section.empty() == false && ns.size() != 0) {
                content_.insert({ section, ns });
                ns.clear();
            }
        }
    }
}
}
