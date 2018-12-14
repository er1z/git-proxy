import xml.dom.minidom
import os
import random
import string
import sys
from subprocess import Popen, PIPE, call as call_process


def get_profile_path(iml_dir):
    deployment_dom_tree = xml.dom.minidom.parse("%s/deployment.xml" % iml_dir)
    collection = deployment_dom_tree.documentElement

    components = collection.getElementsByTagName('component')

    web_server_profile = None

    for c in components:
        if c.getAttribute('name') == 'PublishConfigData':
            if c.hasAttribute('serverName'):
                web_server_profile = c.getAttribute('serverName')
                break

    if web_server_profile is None:
        return None

    webservers_dom_tree = xml.dom.minidom.parse("%s/webServers.xml" % iml_dir)

    components = webservers_dom_tree.documentElement.getElementsByTagName('component')

    for c in components:
        if c.getAttribute('name') == 'WebServers':
            options = c.getElementsByTagName('option')
            for o in options:
                if o.getAttribute('name') == 'servers':
                    servers = o.getElementsByTagName('webServer')
                    for s in servers:
                        if s.getAttribute('name') == web_server_profile:
                            file_transfers = s.getElementsByTagName('fileTransfer')
                            for f in file_transfers:
                                if f.getAttribute('accessType') == 'SFTP' and f.hasAttribute('rootFolder'):
                                    return f.getAttribute('rootFolder'), f.getAttribute('host')


if __name__ == "__main__":

    has_stdin = False
    project_dir = os.getcwd()
    while not os.path.exists("%s/.idea" % project_dir):
        project_dir = os.path.dirname(
            project_dir
        )

        if project_dir.__len__() == 3:
            project_dir = None
            break

    if not project_dir:
        # todo: default host for check
        cmd = "ssh -i host cd /tmp && git %s" % sys.argv[1]

        p = Popen(cmd, stdout=PIPE, stdin=PIPE)
        grep_stdout = p.communicate()[0]
        print(grep_stdout.decode())
        exit(0)

    relative_cwd = os.getcwd()[project_dir.__len__() + 1:]
    relative_cwd = relative_cwd.replace("\\", "/")

    remote_path, host = get_profile_path("%s/.idea" % project_dir)
    remote_cd = "%s/%s" % (remote_path, relative_cwd)

    args = sys.argv[1:]
    target_args = []
    for a in args:
        if a.__len__() > 1 and a[1] == ':' and a[2] == "\\":
            tmp_name = ''.join(random.choice(string.ascii_letters) for i in range(8))
            tmp_path = "/tmp/git_commit_message_%s" % tmp_name  # type: str
            call_process(['scp', a, '%s:%s' % (host, tmp_path)])
            target_args.append(tmp_path)
        elif a == "-" or a == "--stdin":
            has_stdin = True
            target_args.append(a)  # dirty, know
        else:
            target_args.append(a)  # dirty, know

    stdin = b''
    if has_stdin:
        stdin = "\n".join(sys.stdin.readlines())

    cmd = "ssh %s cd %s && git %s" % (host, remote_cd, " ".join(target_args))

    p = Popen(cmd, stdout=PIPE, stdin=PIPE)
    grep_stdout = p.communicate(input=stdin)[0]
    print(grep_stdout)
