#pragma once
#include <string>

#include "../def.h"


namespace windowSave {
	extern std::string settingFile;
	extern json saveWidgetCore(QWidget *parent);
	extern void loadWidgetCore(QWidget *parent, const json& j);
	extern void save();
	extern void load();
}
