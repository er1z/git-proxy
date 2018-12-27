#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#define SI_SUPPORT_IOSTREAMS
#include "config.h"
#include "project.h"
#include "debug.h"
#include "files_queue.h"
#include "process_io.h"

Configuration* config;

std::string get_stdin() {
	int c, i = 0;
	char str[512];

	bool start = true;

	std::string result = "";

	while (true) {

		c = getchar();

		if (start) {
			i = 0;
			memset(str, 0, 512);

			if (c == '\n') {
				break;
			}

		}

		str[i] = c;

		if (i < 511) {
			i++;
		}
		else {
			c = '\n';
		}

		if (c == '\n') {
			start = true;
			result = result + str;
		}
		else {
			start = false;
		}

	}

	return result;
}

bool is_command_remote(int argc, char *argv[]) {

	for (int i = 1; i < argc; i++) {
		if (isprint(argv[i][0])) {
			for (auto local : Configuration::instance()->locals()) {
				if (strcmp(argv[i], local.c_str())==0) {
					debugger("command configured as local", argv[i]);
					return false;
				}
			}
		}
	}

	return true;

}

int main(int argc, char *argv[])
{

	config = Configuration::instance();
	config->load(boost::filesystem::system_complete(argv[0]).parent_path().generic_string() + "\\config.ini");
	
	if (argc > 1) {

		if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "--version") == 0) {
			debugger("quered git version info", "--version");
			ProcessExecResult version_info = execute_remote_command_io(config->default_host(), "git --version", "");
			std::cout << version_info.cout();
			return version_info.exit_code();
		}
	}

	bool process_stdin = false;
	bool command_remote = is_command_remote(argc, argv);

	ProjectInfo project;

	try {
		project = get_project_info();
	}
	catch (ProjectInfoFailedException e) {
		debugger("failed to read project info", e.message());
		std::cerr << e.message();
		return 1;
	}

	TemporaryFilesQueue queue(project);

	std::vector<std::string> arguments;

	for (int i = 1; i < argc; i++) {

		if (strcmp(argv[i], "--stdin") == 0 || strcmp(argv[i], "-") == 0) {
			process_stdin = true;
		}

		if (command_remote && argv[i][1] == ':') {
			arguments.push_back(
				queue.enqueue(argv[i])
			);
		}
		else {
			arguments.push_back(argv[i]);
		}
	}

	queue.send();

	std::string remote_cd = get_remote_cd(project);

	std::string input = "";
	if (process_stdin) {
		debugger("stdin", "processing");
		input = get_stdin();
	}

	ProcessExecResult result = command_remote ? execute_remote_git(project, arguments, input) : execute_local_git(arguments, input);
	
	queue.cleanup();

	std::cout << result.cout();
	std::cerr << result.cerr();

	return result.exit_code();

}


