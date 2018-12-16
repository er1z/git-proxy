#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include "pugixml/pugixml.cpp"
#include "boost/process.hpp"
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include <exception>
#define SI_SUPPORT_IOSTREAMS
#include "SimpleIni.h"


namespace bp = boost::process;

class ConfigurationStruct {

private:
	std::string _default_host;
	std::string _scp_path;
	std::string _ssh_path;

public:
	ConfigurationStruct() {}

	ConfigurationStruct(std::string default_host, std::string scp_path, std::string ssh_path) {
		_default_host = default_host;
		_scp_path = scp_path;
		_ssh_path = ssh_path;
	}

	std::string default_host() {
		return _default_host;
	}

	std::string scp_path() {
		return _scp_path;
	}

	std::string ssh_path() {
		return _ssh_path;
	}
};

ConfigurationStruct config;

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

class TemporaryFilesQueue {

private:
	std::vector<boost::filesystem::path> files;

public:

	void send() {

		std::string arg = "";

		for (auto i : files)
		{
			arg = arg + " " + i.generic_path().generic_string();
		}

		int result = bp::system(config.scp_path() + " " + arg + " dev:/tmp", bp::std_out > bp::null);


	}

	void cleanup() {
		std::string arg = "";

		for (auto i : files)
		{
			arg = arg + " /tmp/" + i.generic_path().filename().generic_string();
		}

		int result = bp::system(config.ssh_path() + " dev rm " + arg, bp::std_out > bp::null);

		files.clear();
	}

	std::string enqueue(std::string path) {
		boost::filesystem::path path_wrapper(path);
		std::string result = path_wrapper.filename().generic_string();

		files.push_back(path_wrapper);

		return "/tmp/" + result;
	}
};

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

ProjectInfo do_get_project_info(std::string project_path) {
	
	std::string work_path = project_path + "\\.idea";

	std::string deployment_path = work_path + "\\deployment.xml";
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(deployment_path.c_str());

	if (!result) {
		throw ProjectInfoFailedException("deployment.xml unreadable");
	}

	pugi::xpath_node_set componentNodes = doc.select_nodes("/project/component[@serverName]");
	pugi::xpath_node component = componentNodes.first();

	std::string server_name = component.node().attribute("serverName").value();

	if (server_name.empty()) {
		throw ProjectInfoFailedException("you should choose a default deployment configuration");
	}

	std::string webservers_path = work_path + "\\webServers.xml";
	pugi::xml_document webserversDoc;
	pugi::xml_parse_result webserversResult = webserversDoc.load_file(webservers_path.c_str());

	if (!webserversResult) {
		throw ProjectInfoFailedException("webservers.xml unreadable");
	}

	pugi::xpath_node_set webserversComponentNodes = webserversDoc.select_nodes("/project/component[@name='WebServers']/option[@name='servers']/webServer[@name='dev']/fileTransfer[@accessType='SFTP']");
	pugi::xpath_node webserversComponent = webserversComponentNodes.first();
	bool empty = webserversComponent.node().empty();

	std::string host = webserversComponent.node().attribute("host").value();
	std::string directory = webserversComponent.node().attribute("rootFolder").value();

	if (host.empty() || directory.empty()) {
		throw ProjectInfoFailedException("host and folder should be supplied");
	}

	return ProjectInfo(host, directory, project_path);
}

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

bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

std::string get_project_path() {

	boost::filesystem::path path = boost::filesystem::system_complete(boost::filesystem::current_path());

	while (!file_exists((path.string() + "\\.idea\\workspace.xml")) && !path.empty()) {
		path = path.parent_path();
	}

	if (path.empty()) {
		throw ProjectPathNotFoundException();
	}

	return path.string();
}

ProjectInfo get_project_info() {

	try {

		std::string working_path = get_project_path();
		
		do {

			boost::replace_all(working_path, "/", "\\");

			try {
				return do_get_project_info(working_path);
			}
			catch (ProjectInfoFailedException i) {
				working_path = boost::filesystem::system_complete(working_path).parent_path().generic_string();
			}

		} while (!working_path.empty());
	}
	catch (ProjectPathNotFoundException p) {
		throw ProjectInfoFailedException("nothing found");
	}

}

std::string get_remote_cd(ProjectInfo project) {

	boost::filesystem::path path = boost::filesystem::system_complete(boost::filesystem::current_path());

	std::string path_str = path.generic_string();
	
	boost::replace_all(path_str, "/", "\\");

	std::string result = path_str.substr(project.realpath().length(), path_str.length() - project.realpath().length());
	boost::replace_all(result, "\\", "/");

	result = project.root() + result;

	return result;
}

ProcessExecResult execute_remote_command(ProjectInfo project, std::vector<std::string> arguments, std::string input) {
	bp::ipstream is;
	bp::ipstream es;
	bp::opstream os;

	PVOID OldValue = NULL;
	Wow64DisableWow64FsRedirection(&OldValue);

	std::string arguments_string;

	for (auto i : arguments) {
		arguments_string = arguments_string + " " + i;
	}

	std::ofstream("D:\\log.txt", std::ios_base::binary | std::ios_base::app) << arguments_string << std::endl;

	bp::child c(config.ssh_path() + " " + project.hostname() + " cd " + project.root() + " && git " +arguments_string, bp::std_out > is, bp::std_in < os, bp::std_err > es);
	Wow64RevertWow64FsRedirection(OldValue);

	os << input << std::endl;
	os.pipe().close();

	std::vector<std::string> cout_data;
	std::vector<std::string> cerr_data;
	std::string line;

	while (std::getline(is, line) && !line.empty()) {
		cout_data.push_back(line);
	}

	while (std::getline(es, line) && !line.empty()) {
		cerr_data.push_back(line);
	}

	c.wait();

	std::string cout_result;
	std::string cerr_result;

	for (const auto& value : cout_data) {
		cout_result = cout_result + value.c_str() + "\n";
	}

	for (const auto& value : cerr_data) {
		cerr_result = cerr_result + value.c_str() + "\n";
	}

	return ProcessExecResult(c.exit_code(), cout_result, cerr_result);
}

ConfigurationStruct get_configuration(std::string config_file) {

	CSimpleIni ini;
	ini.SetUnicode();
	ini.LoadFile(config_file.c_str());

	return ConfigurationStruct(
		ini.GetValue("proxy", "default_host", ""),
		ini.GetValue("proxy", "scp_path", "c:\\Program Files\\Git\\usr\\bin\\scp.exe"),
		ini.GetValue("proxy", "ssh_path", "c:\\Program Files\\Git\\usr\\bin\\ssh.exe")
	);
}

int main(int argc, char *argv[])
{

	try {
		config = get_configuration(boost::filesystem::system_complete(argv[0]).parent_path().generic_string() + "\\config.ini");
	}
	catch (std::string e) {
		std::cerr << e;
		return 1;
	}

	TemporaryFilesQueue queue;
	std::vector<std::string> arguments;

	
	bool process_stdin = false;
	if (argc > 1) {

		//todo: default host - from configuration of the same directory
		if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "--version") == 0) {
			//todo check version method
			//ProcessExecResult version_info = execute_remote_command()
			std::cout << "git version 2.19.1";
			return 0;
		}

		for (int i = 1; i < argc; i++) {

			if (strcmp(argv[i], "--stdin") == 0 || strcmp(argv[i], "-") == 0) {
				process_stdin = true;
			}

			if (argv[i][1] == ':') {
				arguments.push_back(
					queue.enqueue(argv[i])
				);
			}
			else {
				arguments.push_back(argv[i]);
			}
		}
	}

	ProjectInfo project;

	try{
		project = get_project_info();
	}
	catch (ProjectInfoFailedException e) {
		std::cerr << e.message();
		return 1;
	}

	std::string remote_cd = get_remote_cd(project);

	std::string input = process_stdin ? get_stdin() : "";

	std::ofstream("D:\\log.txt", std::ios_base::binary | std::ios_base::app) << input << std::endl;

	ProcessExecResult result = execute_remote_command(project, arguments, input);
	std::cout << result.cout();
	std::cerr << result.cerr();

	return result.exit_code();

}


