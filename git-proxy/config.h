#pragma once

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

class Configuration {

private:
	std::wstring _default_host;
	std::wstring _scp_path;
	std::wstring _ssh_path;
	std::wstring _git_path;
	std::vector<std::string> _locals;
	std::wstring _debug;
	std::wstring _remote_tmp;

	std::vector<std::string> processLocals(std::wstring arg) {
		std::vector<std::string> results;
		boost::split(results, arg, [](char c) {return c == ','; });
		return results;
	}

	Configuration() {}

public:
	
	static Configuration* instance();
	void load(std::string config_file);

	std::string default_host() {
		return std::string(_default_host.begin(), _default_host.end());
	}

	std::string scp_path() {
		return std::string(_scp_path.begin(), _scp_path.end());
	}

	std::string ssh_path() {
		return std::string(_ssh_path.begin(), _ssh_path.end());
	}

	std::vector<std::string> locals() {
		return _locals;
	}

	std::string debug() {
		return std::string(_debug.begin(), _debug.end());
	}

	std::string remote_tmp() {
		return std::string(_remote_tmp.begin(), _remote_tmp.end());
	}

	std::string git_path() {
		return std::string(_git_path.begin(), _git_path.end());
	}

};