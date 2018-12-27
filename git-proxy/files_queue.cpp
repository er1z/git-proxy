#pragma once
#include "pch.h"
#include <string>
#include "config.h"
#include "boost/process.hpp"
#include "files_queue.h"
#include "debug.h"
#include "project.h"
#include "process_io.h"

namespace bp = boost::process;

TemporaryFilesQueue::TemporaryFilesQueue(ProjectInfo project) {
	_project = project;
}

void TemporaryFilesQueue::send() {

	if (files.capacity() == 0) {
		return;
	}

	std::string arg = "";

	for (auto i : files)
	{
		arg = arg + " " + i.generic_path().generic_string();
	}

	std::string command = Configuration::instance()->scp_path() + " " + arg + " " + _project.hostname() + ":" + Configuration::instance()->remote_tmp();
	debugger("sending temporary files", command);
	execute_command_io(command, "");
}

void TemporaryFilesQueue::cleanup() {
	std::string arg = "";

	// dangerous...
	if (Configuration::instance()->remote_tmp().empty() || files.capacity()==0) {
		return;
	}

	for (auto i : files)
	{
		arg = arg + " " + Configuration::instance()->remote_tmp() + "/" + i.generic_path().filename().generic_string();
	}

	std::string command = "rm " + arg;
	debugger("removing temporary files", command);
	execute_remote_command_io(_project.hostname(), command, "");

	files.clear();
}

std::string TemporaryFilesQueue::enqueue(std::string path) {
	boost::filesystem::path path_wrapper(path);
	std::string result = path_wrapper.filename().generic_string();

	files.push_back(path_wrapper);

	std::string result_path = Configuration::instance()->remote_tmp() + "/" + result;
	debugger("enqueued temporary file", result_path);

	return result_path;
}