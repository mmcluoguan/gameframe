#include "shynet/utils/IniConfig.h"
#include <fstream>
#include "shynet/utils/Logger.h"
#include "shynet/utils/StringOp.h"

namespace shynet
{
	namespace utils {
		IniConfig::IniConfig(const char* filename)
		{
			std::ifstream reader;
			reader.open(filename, std::ios::in);
			if (reader.is_open() == false)
			{
				std::ostringstream err;
				err << "open:" << ((filename == nullptr) ? "null" : filename);
				throw SHYNETEXCEPTION(err.str());
			}
			else
			{
				std::string section;
				nodes ns;

				while (!reader.eof())
				{
					std::string::size_type lpos = 0;
					std::string::size_type rpos = 0;
					std::string::size_type epos = 0;
					std::string line, key, value;
					getline(reader, line);
					StringOp::trim(line);
					if (line.empty() == false)
					{
						lpos = line.find('[');
						rpos = line.find(']');
						if (line.npos != lpos && line.npos != rpos && rpos > lpos)
						{
							if (section.empty() == false && ns.size() != 0)
							{
								content_.insert({ section,ns });
								ns.clear();
							}
							section = line.substr(lpos + 1, rpos - 1);
							continue;
						}
						epos = line.find('=');
						if (line.npos != epos)
						{
							key = line.substr(0, epos);
							value = line.substr(epos + 1, line.length() - 1);
							StringOp::trim(key);
							StringOp::trim(value);
						}
						if (section.empty() == false)
						{
							ns.insert({ key,{ section,key,value } });
						}
					}
				}
				reader.close();
				if (section.empty() == false && ns.size() != 0)
				{
					content_.insert({ section,ns });
					ns.clear();
				}
			}
		}

		IniConfig::~IniConfig()
		{
		}
	}
}
