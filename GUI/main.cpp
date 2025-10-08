#include <iostream>
#include <fstream>
#include <filesystem>

#include <QtWidgets/QMainWindow>
#include <QtGui/QShortcut>

#include <yy981/env.h>

#include "../master.h"
#include "cui.h"
#include "gui.h"
#include "ui_main.h"
#include "../CUI/ui.h"
#include "../CUI/text.h"

#include "widgets/fileButton.h"

namespace fs = std::filesystem;
Ui::MainWindow* ui = nullptr;
QMainWindow* w = nullptr;
json p_kek;


struct VER {
	constexpr VER(const uint16_t& major, const uint16_t& minor, const uint16_t& patch = 0): major(major), minor(minor), patch(patch) {}
	
	uint16_t gen{7}, major, minor, patch;
	
	constexpr std::string str() const {
		return "v"+std::to_string(major)+"."+std::to_string(minor)+(patch==0? "": "."+patch);
	}
	
	constexpr uint64_t num() const {
		return (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}
	constexpr uint64_t numALL() const {
		return (uint64_t)gen<<(16*3) | (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}
};
constexpr VER ver(0,2);

void showMessage(const std::string& msg) {
	QWidget* w = new QWidget;
	w->setWindowTitle("通知");

	QVBoxLayout* layout = new QVBoxLayout(w);
	QLabel* label = new QLabel(QString::fromStdString(msg));
	layout->addWidget(label);

	QPushButton* button = new QPushButton("OK");
	layout->addWidget(button);
	QObject::connect(button, &QPushButton::clicked, w, &QWidget::close);
	w->setLayout(layout);
	w->show();
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

namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	
	void textProc(const std::string& text) {
		std::string out;
		
		int index = ui->selectKey->currentIndex();
		std::string kid = ui->selectKey->itemData(index).toString().toStdString();
		
		if (ui->encMode->isChecked()) {
			// out = EAD7::enc(kek,text,mkid,kid);
		} else {
			
		}
		ui->out->setPlainText(QString::fromStdString(out));
	}
	
	void fileProc(const std::string& text) {
		
	}
	
	void run() {
		std::string text;
		switch (inp_from) {
			case INP_FROM::null: u::stat("入力元が特定できません"); return;
			case INP_FROM::line: text = ui->inp_line->text().toStdString(); break;
			case INP_FROM::multi: text = ui->inp_multi->toPlainText().toStdString(); break;
			case INP_FROM::file: text = ui->inp_file_path->text().toStdString(); break;
		}
		if (inp_from == INP_FROM::file) fileProc(text); else textProc(text);
	}
	
}

void loadKeyCombobox() {
	if (!fs::exists(path::p_kek)) {
		ui->selectKey->addItem(QString::fromStdString("Keyリストを配布KEKファイルから登録してください"));
		for (QWidget* child : w->findChildren<QWidget*>()) {
			if (child != ui->dst_kek) child->setEnabled(false);
		}
		ui->selectKey->setStyleSheet("background-color: red; color: black; font-weight: bold;");
		return;
	}
	p_kek = readJson(path::p_kek);
	
	json raw_kek = decPKEK(p_kek);
	u::log("Keyリスト(P_KEK)読み込み完了   最終更新日時: " + convUnixTime(raw_kek["meta"]["last_updated"].get<uint64_t>()));
	KIDIndex index = createKIDIndex(raw_kek);
	for (const auto [label,kid]: index) {
		ui->selectKey->addItem(QString::fromStdString(label),QString::fromStdString(kid));
	}
	delm(raw_kek);
}

void GUI() {
	windowSave::settingFile = SD+"GUI_setting.json";	
	ui->log->setVisible(false);
	w->show();
	
	ui->credit->setStyleSheet("color: tomato;");
	ui->credit->setText(QString::fromStdString("EAD7 " + ver.str() + " (C) 2025 yy981"));
	if (fs::exists(SDM)) {
		ui->adminMode->setEnabled(true);
		ui->adminMode->setChecked(true);
	}
	
	QObject::connect(app, &QCoreApplication::aboutToQuit, windowSave::save);
	QShortcut shortcut_inp_multi(QKeySequence("Alt+Return"), ui->inp_multi);
	QObject::connect(ui->run, &QPushButton::clicked, mw::run);
	QObject::connect(&shortcut_inp_multi, &QShortcut::activated, ui->run, &QPushButton::animateClick);
	QObject::connect(ui->inp_line, &QLineEdit::returnPressed, []{mw::inp_from=INP_FROM::line; ui->run->animateClick();});
	QObject::connect(ui->inp_file_path, &QLineEdit::returnPressed, []{mw::inp_from=INP_FROM::file; ui->run->animateClick();});
	QObject::connect(ui->inp_file_button, &FileButton::fileSelected, [](const QString& selectedFile){
		mw::inp_from=INP_FROM::file;
		ui->inp_file_path->setText(selectedFile);
		ui->run->animateClick();
	});
	QObject::connect(ui->log_checkbox, &QCheckBox::checkStateChanged, ui->log, &QPlainTextEdit::setVisible);
	
	loadKeyCombobox();
	
	if (fs::exists(windowSave::settingFile)) windowSave::load();

	u::log("ui setup完了");
	
}

void crashReport(const std::string& text) {
	std::string crash_log_path = getEnv("tmp")+"/EAD7_crash.log";
	std::ofstream(crash_log_path) << t::banner << "\n\n\n\n\n" << convUnixTime(getUnixTime())
								  << ":   " << text << "\n";
	openFile(crash_log_path);
}

int GUI_interface() {
	bool crashed = false;
	std::string err;
	QString log;
	for (int i = 1; i <= 100; ++i) {
		try {
			w = new QMainWindow;
			ui = new Ui::MainWindow;
			ui->setupUi(w);
			if (!crashed) u::log("EAD GUI 起動"); else {
				crashed = false;
				ui->log->setPlainText(log);
				u::stat("RuntimeError: " + err);
				u::log("RuntimeError: " + err);
				u::log("EAD GUI 再起動");
			}
			GUI(); // Core
			int result = app->exec();
			delete w; // QThreadStorageエラー対策
			return result;
		} catch (const std::runtime_error& e) {
			crashed = true;
			err = e.what();
			log = ui->log->toPlainText();
			w->close();
			continue;
		} catch (const std::exception& e) {
			crashReport("深刻な例外が発生したため、プログラムを終了しました\nstd::exception::what(): " + std::string(e.what()));
			return 1;
		} catch (...) {
			crashReport("不明な例外が発生したため、プログラムを終了しました");
			return 2;
		}
	}
	crashReport("RuntimeErrorが100回発生したため明らかな異常と判断し、プログラムを終了しました");
	return 50;
}
