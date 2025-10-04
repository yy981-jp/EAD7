#pragma once
#include <string>
#include <vector>


struct Entry {
	std::string label; // 表示用
	std::string id;	// 内部用
	bool checked;
	Entry(const std::string& label, const std::string& id, bool checked = false): label(label), id(id), checked(checked) {}
};


extern std::vector<std::string> selectItem(const std::vector<Entry>& entries);
extern void gmain();