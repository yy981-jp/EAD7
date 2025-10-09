#include <iostream>
#include <fstream>
#include <filesystem>

#include <QtWidgets/QMainWindow>
#include <QtGui/QShortcut>

#include <yy981/env.h>

#include "../master.h"
#include "def.h"
#include "cui.h"
#include "gui.h"
#include "ui_main.h"
#include "../CUI/ui.h"
#include "../CUI/text.h"
#include "windowSave.h"

#include "widgets/fileButton.h"

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

namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	
	void import_dst_kek(const QString& qstr) {
		std::string str = qstr.toStdString();
		if (str.ends_with(".e7")) {
			fs::path p = str;
			if (fs::exists(p)) {
				FDat f = getFileType(p);
				switch (f.type) {
					case FSType::dst_kek: {
						std::string pass = prompt("DST.KEKファイルのパスワード: ");
						json raw_kek = decDstKEK(pass,f.json);
						json p_kek = encPKEK(raw_kek);
						writeJson(p_kek,path::p_kek);
						u::stat("P_KEK更新完了\n");
						delm(pass,raw_kek);
						throw std::runtime_error("再起動信号(P_KEK再読み込み)");
					} break;
					default: throw std::runtime_error("E7ファイルではありますが、形式が不正です");
				}
			}
		}
		u::stat("dst_kekとして入力されたファイルはe7ファイルではありません(ファイル拡張子で判断)");
	}
	
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
		w->close();
		delete fb;
		fb = new FileButton;
		fb->show();
		QObject::connect(ui->inp_file_button, &FileButton::fileSelected, mw::import_dst_kek);
		return;
	}
	PKEK = readJson(path::p_kek);
	
	json raw_kek = decPKEK(PKEK);
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

int GUI_interface() {
	bool crashed = false;
	std::string err;
	QString log, statText;
	for (int i = 1; i <= 100; ++i) {
		try {
			delete w;
			delete ui;
			w = new QMainWindow;
			ui = new Ui::MainWindow;
			ui->setupUi(w);
			if (!crashed) u::log("EAD GUI 起動"); else {
				crashed = false;
				ui->log->setPlainText(log);
				ui->statusbar->showMessage(statText,60*1000); // 1分
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
			statText = ui->statusbar->currentMessage();
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
