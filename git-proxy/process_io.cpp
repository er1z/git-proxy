#pragma once
#include "pch.h"
#include "process_io.h"
#include "boost/process.hpp"
#include "project.h"
#include "debug.h"
#include "config.h"

namespace bp = boost::process;

ProcessExecResult execute_command_io(std::string command, std::string input) {
	bp::ipstream is;
	bp::ipstream es;
	bp::opstream os;

	PVOID OldValue = NULL;
	Wow64DisableWow64FsRedirection(&OldValue);

	std::string arguments_string;

	debugger("executing process", command);

	bp::child c(command, bp::std_out > is, bp::std_in < os, bp::std_err > es);

	if (!input.empty()) {
		debugger("stdin", input);
		os << input << std::endl;
	}
	else {
		os << std::endl << std::endl;
	}

	os.pipe().close();

	Wow64RevertWow64FsRedirection(OldValue);

	std::vector<std::string> cerr_data;
	std::string line;

	std::string cout_result = "";

	int i;
	while (!is.eof()) {
		i = is.get();

		if (i == EOF) {
			break;
		}

		cout_result.append(1, char(i));
	}

	while (c.running() && std::getline(es, line) && !line.empty()) {
		cerr_data.push_back(line);
	}

	c.wait();

	std::string cerr_result;

	// todo: its not necessary
	for (const auto& value : cerr_data) {
		cerr_result = cerr_result + value.c_str() + "\n";
	}

	debugger("response", cout_result);
	debugger("stderr", cerr_result);

	return ProcessExecResult(c.exit_code(), cout_result, cerr_result);
}

ProcessExecResult execute_remote_command_io(std::string host, std::string command, std::string input) {

	std::string command_line = Configuration::instance()->ssh_path() + " " + host + " " + command;

	return execute_command_io(command_line, input);
}

ProcessExecResult execute_remote_git(ProjectInfo project, std::vector<std::string> arguments, std::string input) {

	std::string arguments_string;
	for (auto i : arguments) {
		arguments_string.append(" " + i);
	}

	std::string command = "\"cd " + project.root() + " && git --no-pager" + arguments_string + "\"";

	return execute_remote_command_io(project.hostname(), command, input);

}

ProcessExecResult execute_local_git(std::vector<std::string> arguments, std::string input) {
	std::string arguments_string;
	for (auto i : arguments) {
		arguments_string.append(" " + i);
	}

	debugger("local git", Configuration::instance()->git_path());
	
	std::string command = Configuration::instance()->git_path()+" --no-pager" + arguments_string;

	return execute_command_io(command, input);
}