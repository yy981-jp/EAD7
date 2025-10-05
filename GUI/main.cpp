#include <iostream>
#include <fstream>
#include <filesystem>

#include <QtWidgets/QMainWindow>
#include <QtGui/QShortcut>

#include "../master.h"
#include "cui.h"
#include "ui_main.h"
#include "../CUI/ui.h"

#include "widgets/fileButton.h"

namespace fs = std::filesystem;
Ui::MainWindow* ui = nullptr;
QMainWindow* w = nullptr;


namespace u {
	void setPrg(const int& v) {
		ui->progressBar->setValue(v);
	}
	void log(const std::string& str) {
		ui->log->appendPlainText(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str));
	}
	void stat(const std::string& str) {
		ui->statusbar->showMessage(QString::fromStdString(str));
	}
}

namespace windowSave {
	std::string settingFile;
	
	json saveWidgetCore(QWidget *parent) {
		json j;
		for (auto w : parent->findChildren<QWidget*>()) {
			std::string name = w->objectName().toStdString();
			if (name.empty()) continue;

			if (auto cb = qobject_cast<QCheckBox*>(w))			j[name] = cb->isChecked();
			else if (auto combo = qobject_cast<QComboBox*>(w))	j[name] = combo->currentIndex();
		}
		return j;
	}

	void loadWidgetCore(QWidget *parent, const json& j) {
		for (auto w : parent->findChildren<QWidget*>()) {
			std::string name = w->objectName().toStdString();
			if (name.empty() || !j.contains(name)) continue;

			if (auto cb = qobject_cast<QCheckBox*>(w))			cb->setChecked(j[name]);
			else if (auto combo = qobject_cast<QComboBox*>(w))	combo->setCurrentIndex(j[name]);
		}
	}
	
	void save() {
		json setting;
		setting["MainWindow"] = saveWidgetCore(w);
		writeJson(setting,settingFile);
	}

	void load() {
		json setting = readJson(settingFile);
		loadWidgetCore(w,setting["MainWindow"]);
	}
}

enum class INP_FROM {
	null, line, multi, file
};

namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	void run() {
		
	}
}

int GUI() {
	windowSave::settingFile = SD+"GUI_setting.json";
	w = new QMainWindow;
	ui = new Ui::MainWindow;
	ui->setupUi(w);
	u::log("EAD GUI 起動");

	ui->log->setVisible(false);
	w->show();

	if (fs::exists(SDM)) {
		ui->adminMode->setEnabled(true);
		ui->adminMode->setChecked(true);
	}
	
	QObject::connect(app, &QCoreApplication::aboutToQuit, windowSave::save);
	QShortcut shortcut_inp_multi(QKeySequence("Alt+Return"), ui->inp_multi);
	QObject::connect(&shortcut_inp_multi, &QShortcut::activated, ui->run, &QPushButton::animateClick);
	QObject::connect(ui->inp_line, &QLineEdit::returnPressed, []{mw::inp_from=INP_FROM::line; ui->run->animateClick();});
	QObject::connect(ui->inp_file_path, &QLineEdit::returnPressed, []{mw::inp_from=INP_FROM::file; ui->run->animateClick();});
	QObject::connect(ui->inp_file_button, &FileButton::fileSelected, [](const QString& selectedFile){
		mw::inp_from=INP_FROM::file;
		ui->inp_file_path->setText(selectedFile);
		ui->run->animateClick();
	});
	QObject::connect(ui->log_checkbox, &QCheckBox::checkStateChanged, ui->log, &QPlainTextEdit::setVisible);
	
	if (fs::exists(windowSave::settingFile)) windowSave::load();

	u::log("ui setup完了");
	
	int result = app->exec();
	delete w; // QThreadStorageエラー対策
	return result;
}