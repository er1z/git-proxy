#pragma once
#include "pch.h"
#include "project.h"
#include "pugixml/pugixml.cpp"
#include "boost/filesystem.hpp"
#include "debug.h"
#include <boost/algorithm/string.hpp>

bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

ProjectInfo do_get_project_info(std::string project_path) {

	std::string work_path = project_path + "\\.idea";

	debugger("searching project", work_path);

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

	debugger("server name", server_name);

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

	debugger("remote path", directory);

	return ProjectInfo(host, directory, project_path);
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

	debugger("project path relative to root", result);

	return result;
}