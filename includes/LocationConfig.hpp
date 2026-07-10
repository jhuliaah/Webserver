#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig {
    public:
		LocationConfig();
		~LocationConfig();

		std::string					_path;
		std::vector<std::string>	_methods;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		std::string					_cgi_extension;
		std::string					_cgi_path;
};

#endif