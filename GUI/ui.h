#pragma once
#include <string>
#include <vector>


struct Entry {
    std::string label; // 表示用
    std::string id;    // 内部用
};


extern std::vector<std::string> selectItem(const std::vector<Entry>& entries);
extern void gmain();