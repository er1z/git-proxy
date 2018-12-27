#pragma once

#include "pch.h"
#include "config.h"
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

void debugger(std::string label, std::string str) {

	if (Configuration::instance()->debug().length() == 0) {
		return;
	}

	boost::posix_time::ptime date_time = boost::posix_time::microsec_clock::universal_time();
	std::string line = to_simple_string(date_time) + " - " + label + ": " + str + "\n";

	std::ofstream(Configuration::instance()->debug().c_str(), std::ios_base::out | std::ios_base::app) << line;

}