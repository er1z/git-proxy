#pragma once
#include "pch.h"
#include "boost/filesystem.hpp"
#include "project.h"

class TemporaryFilesQueue {

private:
	ProjectInfo _project;
	std::vector<boost::filesystem::path> files;

public:
	TemporaryFilesQueue(ProjectInfo project);
	void send();
	void cleanup();
	std::string enqueue(std::string path);
};