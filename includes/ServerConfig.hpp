#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <map>
#include <string>

class ServerConfig {
	private:

		int							_port;
		std::string					_host;
		std::string					_name;
		std::string					_root;
		std::string					_index;
		size_t						_max_body_size;
		std::map<int, std::string>	_error_pages;
		std::vector<LocationConfig> _locations;

	public:
		ServerConfig();
		~ServerConfig();

		int getPort() const { return _port; }
		const std::string& getHost() const { return _host; }
		const std::string& getName() const { return _name; }
		const std::string& getRoot() const { return _root; }
		const std::string& getIndex() const { return _index; }
		size_t getMaxBodySize() const { return _max_body_size; }
		const std::map<int, std::string>& getErrorPages() const { return _error_pages; }
		const std::vector<LocationConfig>& getLocations() const { return _locations; }

		void setPort(int port) { _port = port; }
		void setHost(const std::string& host) { _host = host; }
		void setName(const std::string& name) { _name = name; }
		void setRoot(const std::string& root) { _root = root; }
		void setIndex(const std::string& index) { _index = index; }
		void setMaxBodySize(size_t size) { _max_body_size = size; }
		void setErrorPages(const std::map<int, std::string>& errorPages) { _error_pages = errorPages; }
		void addLocation(const LocationConfig& location) { _locations.push_back(location); }
};

#endif