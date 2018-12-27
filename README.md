# git-proxy
An utility to use remote git instance — helpful when you keep sources on VM and mount it via CIFS on host.

## how it works
For example, Docker for Windows has a completely pathetic performance on I/O, so I found out a workflow on Windows which allows to work on projects almost painless:

1. Create a VM with project runtime, store project files **on** this machine,
2. Set up a CIFS/Samba share and mount it on Windows side.

Then open this project using an IntelliJ tool, eg. PHPStorm. Indexing performance is acceptable, but git updates/commits/diffs are very slow. So I came across idea to proxy IntelliJ's git requests to VM where it runs under native filesystem, not mounted one. All you need is setup this tool and forget.

Basicly, `git-proxy`:
1. Proxies stdin/arguments to defined VM (according to default deployment configuration),
2. Sniffs paths to files (especially on `git commit` message) and makes sure they are available on VM (using SCP, executing git and cleaning up),
3. Returns stdout to IntelliJ.

## usage

1. Install [Git for Windows](https://git-scm.com/download/win),
1. Create ~/.ssh/config with an entry pointing to your VM:
```
Host dev
	User me
	IdentityFile...
```
Make sure it's loginless, eg. run `ssh-agent` or passwordless key login.
2. Create a project within your virtual machine and configure deployment for project. Use the same name as in SSH configuration, make it default (as the `git-proxy` uses the default connection to map directories). Make sure you fill up the remote directory field:
![Deployment configuration](https://github.com/er1z/git-proxy/raw/master/docs/deployment_config.png "Configuration")
3. Extract release archive anywhere you want and modify `config.ini` file:
```
[proxy]
default_host=dev
```
4. Change IntelliJ git binary path to point on executable:
![Git path](https://github.com/er1z/git-proxy/raw/master/docs/git_config.png "Git path")

## configuration
Put `config.ini` file in the same directory as the executable:
```ini
[proxy]
;debug= ; eg. D:\\log.txt store some debug information if something works wrong
default_host=dev	; default host used for git --version
;scp_path=c:\\Program Files\\Git\\usr\\bin\\scp.exe	; custom SCP binary path
;ssh_path=c:\\Program Files\\Git\\usr\\bin\\ssh.exe ; custom SSH binary path
;git_path=c:\\Program Files\\Git\\bin\\git.exe ; custom git binary path
;locals=log	; comma-separated list of commands executed local
;remote_tmp=/tmp	; writable directory for translating paths
```

## building
It was created using Visual Studio 2017 and Boost libraries. Pull this repository, install packages and download git submodules:

```
git clone --recurse-submodules -j8 https://github.com/er1z/git-proxy.git
```

## disclaimer
I'm not a C++ developer and take **no responsibility** on any data lose. You're now aware. 

But honestly, I'm using this tool daily and nothing is lost. Feel free to experiment with code and post issues, submit pull requests.