#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class LocationConfig {
	private:
		std::string					_path;
		std::vector<std::string>	_methods;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		std::string					_cgi_extension;
		std::string					_cgi_path;
		std::string					_returnPath;
		int							_returnCode;
		std::map<int, std::string>	_error_pages;

	public:
		LocationConfig();
		~LocationConfig();

		// getters
		const std::string& getPath() const { return _path; }
		const std::vector<std::string>& getMethods() const { return _methods; }
		const std::string& getRoot() const { return _root; }
		const std::string& getIndex() const { return _index; }
		bool getAutoindex() const { return _autoindex; }
		const std::string& getCgiExtension() const { return _cgi_extension; }
		const std::string& getCgiPath() const { return _cgi_path; }
		const std::string& getReturnPath() const { return _returnPath; }
		int getReturnCode() const { return _returnCode; }
		const std::map<int, std::string>& getErrorPages() const { return _error_pages; }

		// setters
		void setPath(const std::string& v) { _path = v; }
		void setMethods(const std::vector<std::string>& v) { _methods = v; }
		void setRoot(const std::string& v) { _root = v; }
		void setIndex(const std::string& v) { _index = v; }
		void setAutoindex(bool v) { _autoindex = v; }
		void setCgiExtension(const std::string& v) { _cgi_extension = v; }
		void setCgiPath(const std::string& v) { _cgi_path = v; }
		void setReturnPath(const std::string& v) { _returnPath = v; }
		void setReturnCode(int v) { _returnCode = v; }
		void setErrorPages(const std::map<int, std::string>& v) { _error_pages = v; }

};
#endif