#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <vector>

#include <yy981/env.h>

#include "../../src/version.h"

namespace fs = std::filesystem;

VER cver("../ver.csv");
const fs::path Dir = fs::current_path()/"..";
const std::string TMP = getEnv("tmp");

void cmd(const std::string& command) {
    if (std::system(command.c_str())) {
        throw std::runtime_error("コマンドの実行に失敗しました: " + command);
    }
}

void confirmationPrompt() {
    std::cout << "本当に実行しますか? (y/n): ";
    char response;
    std::cin >> response;
    if (response != 'y' && response != 'Y') {
        std::cout << "操作がキャンセルされました\n";
        std::exit(0);
    }
}

int main() {
    std::cout << "アップデート種類を選択: m(a)jor/m(i)nor/(p)atch or (e)xit: ";
    char choice;
    std::cin >> choice;
    
    VER oldVer = cver;
    switch (choice) {
        case 'a': // major
            confirmationPrompt();
            cver.major += 1;
            cver.minor = 0;
            cver.patch = 0;
            break;
        case 'i': // minor
            confirmationPrompt();
            cver.minor += 1;
            cver.patch = 0;
            break;
        case 'p': // patch
            cver.patch += 1;
            break;
        case 'e':
            std::cout << "終了します\n";
            return 0;
        default:
            std::cout << "無効な選択肢です 終了します\n";
            return 1;
    }
    cmd("title EAD7" + oldVer.str() + " -> " + cver.str());

    std::cout << "mainBranch-commitMessage: ";
    std::string commitMessage;
    std::cin >> commitMessage;
    
    // git
    fs::current_path(Dir);
    cmd("git tag" + cver.str());
    cmd("git switch main");
    cmd("git merge" + cver.str());
    cmd("git commit -m \"" + commitMessage + "\"");
    cmd("git switch develop");

    // build
    fs::current_path(Dir/"build");
    cmd("cmake .. -DCMAKE_BUILD_TYPE=Release");
    cmd("Ninja");
    fs::copy(Dir/"build"/"EAD7.exe", Dir/"release"/"EAD7.exe", fs::copy_options::overwrite_existing);
    cmd("cmake .. -DCMAKE_BUILD_TYPE=Debug");
    
    // collect
    cmd("dll -modules -cache "+(Dir/release).string()+" > "+TMP+"/EAD7_dst_dlllist.txt");
    std::ifstream ifs(TMP+"/EAD7_dst_dlllist.txt");
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.starts_with("[Environment] ")) continue;
        line.erase(0, 14); // "[Environment] "部分を削る
        size_t pos = line.find(" ");
        line.erase(pos); // 最初の空白以降を削る
        line = "c:/msys64/mingw64/bin/" + line;

        fs::copy(line, Dir/"release"/fs::path(line).filename(), fs::copy_options::overwrite_existing);
    }
}