#pragma once
#include "pch.h"
#include <string>

class ProjectInfoFailedException : public std::exception
{

private:
	std::string _message;

public:
	ProjectInfoFailedException(std::string message) {
		_message = message;
	}

	std::string message() {
		return _message;
	}
};

class ProjectPathNotFoundException : public std::exception {};

class ProjectInfo {

private:
	std::string _hostname;
	std::string _root;
	std::string _realpath;

public:
	ProjectInfo() {

	}

	ProjectInfo(std::string hostname, std::string root, std::string realpath) {
		_hostname = hostname;
		_root = root;
		_realpath = realpath;
	}

	std::string hostname() {
		return _hostname;
	}

	std::string root() {
		return _root;
	}

	std::string realpath() {
		return _realpath;
	}

};

ProjectInfo do_get_project_info(std::string project_path);
std::string get_project_path();
ProjectInfo get_project_info();
std::string get_remote_cd(ProjectInfo project);