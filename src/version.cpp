#include "version.h"

#include <fstream>
#include <vector>

#include <yy981/string.h>


void VER::loadCSV() {
	std::string path = csvPath;
	if (!fs::exists(path) && fs::current_path().filename().string() == "build") path = (fs::path("..") / csvPath).string();
	std::ifstream ifs(path);
	if (!ifs) throw std::runtime_error("VER::loadCSV(): ifs " + path);
	std::string line;
	std::getline(ifs,line);
	const auto list = st::spliti(line,",");
	if (list.size() != 2 && list.size() != 3) throw std::runtime_error("VER::loadCSV(): invalid format: commaNum: " + std::to_string(list.size()));
	major = list[0];
	minor = list[1];
	if (list.size() == 3) patch = list[2];
}

VER ver;
