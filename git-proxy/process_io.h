#pragma once
#include "pch.h"
#include "project.h"
#include <string>
#include <vector>

class ProcessExecResult {

private:
	std::string _cout;
	std::string _cerr;
	int _exit_code;

public:
	ProcessExecResult(int exit_code, std::string cout, std::string cerr) {
		_cout = cout;
		_cerr = cerr;
		_exit_code = exit_code;
	}

	std::string cout() {
		return _cout;
	}

	std::string cerr() {
		return _cerr;
	}

	int exit_code() {
		return _exit_code;
	}

};

ProcessExecResult execute_command_io(std::string command, std::string input);
ProcessExecResult execute_remote_command_io(std::string host, std::string command, std::string input);
ProcessExecResult execute_remote_git(ProjectInfo project, std::vector<std::string> arguments, std::string input);
ProcessExecResult execute_local_git(std::vector<std::string> arguments, std::string input);