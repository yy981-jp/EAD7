#pragma once
#include <string>

#include "../def.h"


namespace windowSave {
	extern std::string settingFile;
	json saveWidgetCore(QWidget *parent);
	void loadWidgetCore(QWidget *parent, const json& j);
	void save();
	void load();
}
