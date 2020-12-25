//
// Created by xiaozhuai on 2019/12/13.
//

#include "OdourlessUtils.h"
#include "FileSystemHelper.h"
#include "ProcessHelper.h"
#include "tinyformat.h"

#include <array>

bool OdourlessUtils::checkSIPEnabled() {
    auto res = shellExec("/usr/bin/csrutil status", false);
    if(!res.suc) return false;
    return res.output.find("disabled") == std::string::npos;
}

bool OdourlessUtils::daemonInstalled() {
    return FileSystemHelper::exists("/Library/LaunchDaemons/odourless-daemon.plist");
}

bool OdourlessUtils::daemonRunning() {
    return ProcessHelper::getPidByProcessPath(ODOURLESS_INSTALL_PATH "/Contents/Resources/bin/odourless-daemon") != -1;
}

ExecResult OdourlessUtils::installDaemon() {
    return shellExec(ODOURLESS_INSTALL_PATH "/Contents/Resources/install-daemon", true);
}

ExecResult OdourlessUtils::uninstallDaemon() {
    return shellExec(ODOURLESS_INSTALL_PATH "/Contents/Resources/uninstall-daemon", true);
}

ExecResult OdourlessUtils::startDaemon() {
    return shellExec(ODOURLESS_INSTALL_PATH "/Contents/Resources/start-daemon", true);
}

ExecResult OdourlessUtils::stopDaemon() {
    return shellExec(ODOURLESS_INSTALL_PATH "/Contents/Resources/stop-daemon", true);
}

ExecResult OdourlessUtils::restartDaemon() {
    return shellExec(ODOURLESS_INSTALL_PATH "/Contents/Resources/restart-daemon", true);
}

ExecResult OdourlessUtils::showDaemonLog() {
    return shellExec("open -a console \"" CAGE_DIRECTORY_PATH "/daemon.log\"", false);
}

ExecResult OdourlessUtils::showInjectLog() {
    return shellExec("open -a console \"" CAGE_DIRECTORY_PATH "/inject.log\"", false);
}

ExecResult OdourlessUtils::shellExec(const std::string &shellPath, bool sudo) {
    std::array<char, 1024> buffer{};
    std::string result;
    std::string cmd;

    if (sudo) {
        cmd = tfm::format(R"("%s/Contents/Resources/sudo-prompt" "%s" 2>&1)", ODOURLESS_INSTALL_PATH, shellPath);
    } else {
        cmd = tfm::format(R"(/bin/bash -c "%s" 2>&1)", shellPath);
    }

    auto pipe = popen(cmd.c_str(), "r");

    if (!pipe) {
        return {
                .ret = 1,
                .suc = false,
                .output = "popen() failed!"
        };
    }

    while (!feof(pipe)) {
        if (fgets(buffer.data(), 1024, pipe) != nullptr) {}
            result += buffer.data();
    }

    auto rc = pclose(pipe);

    if (sudo) {
        // I don't why osascript print \r but not \n
        std::replace(result.begin(), result.end(), '\r', '\n');
    }

    return {
            .ret = rc,
            .suc = rc == 0,
            .output = result
    };
}
