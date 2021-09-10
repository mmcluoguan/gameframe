#include "shynet/IniConfig.h"
#include <fstream>
#include "shynet/Utility.h"
#include "shynet/Logger.h"

namespace shynet
{
	IniConfig::IniConfig(const char* filename)
	{
		std::ifstream reader;
		reader.open(filename, std::ios::in);
		if (reader.is_open() == false)
		{
			LOG_ERROR << "open:" << ((filename == nullptr) ? "null" : filename);
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
				Utility::trim(line);
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
						Utility::trim(key);
						Utility::trim(value);
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
