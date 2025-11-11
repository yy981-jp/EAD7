#include <QtWidgets/QWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QRadioButton>

#include "windowSave.h"
#include "../master.h"
#include "../def.h"
#include "def.h"


	std::string windowSave::settingFile;
	
	json windowSave::saveWidgetCore(QWidget *parent) {
		json j;
		for (auto w : parent->findChildren<QWidget*>()) {
			std::string name = w->objectName().toStdString();
			if (name.empty()) continue;

			if (auto cb = qobject_cast<QCheckBox*>(w))				j[name] = cb->isChecked();
			else if (auto combo = qobject_cast<QComboBox*>(w))		j[name] = combo->currentIndex();
			else if (auto radio = qobject_cast<QRadioButton*>(w))	j[name] = radio->isChecked();
		}
		return j;
	}

	void windowSave::loadWidgetCore(QWidget *parent, const json& j) {
		for (auto w : parent->findChildren<QWidget*>()) {
			std::string name = w->objectName().toStdString();
			if (name.empty() || !j.contains(name)) continue;

			if (auto cb = qobject_cast<QCheckBox*>(w))				cb->setChecked(j[name]);
			else if (auto combo = qobject_cast<QComboBox*>(w))		combo->setCurrentIndex(j[name]);
			else if (auto radio = qobject_cast<QRadioButton*>(w))	radio->setChecked(j[name]);
		}
	}
	
	void windowSave::save() {
		json setting;
		setting["MainWindow"] = saveWidgetCore(w);
		writeJson(setting,settingFile);
	}

	void windowSave::load() {
		json setting = readJson(settingFile);
		loadWidgetCore(w,setting["MainWindow"]);
	}
