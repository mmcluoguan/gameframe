#pragma once
#include <string>
#include <sstream>
#include <unordered_map>
#include "shynet/Basic.h"

namespace shynet
{
	namespace utils {
		class IniConfig final : public Nocopy
		{
			struct node
			{
				std::string section;
				std::string key;
				std::string value;
			};

			typedef std::string section;
			typedef std::string key;
			typedef node value;
			typedef std::unordered_map<key, value> nodes;
			typedef std::unordered_map<section, nodes> serctions;
		public:
			static constexpr const char* classname = "IniConfig";
			explicit IniConfig(const char* filename);
			~IniConfig();

			template<class IN, class OUT >
			OUT get(section sec, key k, const IN& defv) const
			{
				const auto& siter = content_.find(sec);
				if (siter == content_.end())
				{
					return defv;
				}
				else
				{
					const auto& niter = siter->second.find(k);
					if (niter == siter->second.end())
					{
						return defv;
					}
					else
					{
						std::stringstream istream;
						istream << niter->second.value;
						OUT t;
						istream >> t;
						if (istream.eof() && !istream.fail())
						{
							return t;
						}
						else
						{
							return defv;
						}
					}
				}
				return defv;
			}
		private:
			serctions content_;
		};
	}
}