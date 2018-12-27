#include "pch.h"
#include "config.h"
#include "simpleini/SimpleIni.h"


void Configuration::load(std::string config_file) {

	CSimpleIni ini;
	ini.SetUnicode();
	ini.LoadFile(config_file.c_str());

	_default_host = ini.GetValue(L"proxy", L"default_host", L"");
	_scp_path = ini.GetValue(L"proxy", L"scp_path", L"c:\\Program Files\\Git\\usr\\bin\\scp.exe");
	_ssh_path = ini.GetValue(L"proxy", L"ssh_path", L"c:\\Program Files\\Git\\usr\\bin\\ssh.exe");
	_git_path = ini.GetValue(L"proxy", L"git_path", L"c:\\Program Files\\Git\\bin\\git.exe");
	_debug = ini.GetValue(L"proxy", L"debug", L"");
	_locals = processLocals(
		ini.GetValue(L"proxy", L"locals", L"log")
	);
	_remote_tmp = ini.GetValue(L"proxy", L"remote_tmp", L"/tmp");
}

Configuration* Configuration::instance()
{
	static Configuration instance;

	return &instance;
}
