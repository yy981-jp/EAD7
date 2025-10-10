#include <iostream>
#include <fstream>
#include <filesystem>

#include <QtWidgets/QMainWindow>
#include <QtGui/QShortcut>
#include <QtGui/QClipboard>

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

namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	
	void setInpFrom(const INP_FROM& inp_from_new) {
		inp_from = inp_from_new;
		
		int index = ui->inp_from->findData(QVariant::fromValue(inp_from));
		if (index != -1) ui->inp_from->setCurrentIndex(index);
	}
	
	void import_dst_kek(const QString& qstr, bool from_kek_window = false) {
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
						if (from_kek_window) fb->close();
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
		if (kid.empty()) kid = ui->selectKey->currentText().toStdString(); // 管理者モード手動入力対応
		if (kid.empty()) {
			u::stat("使用する鍵を選択してください");
			return;
		}
		json raw_kek = decPKEK(PKEK);
		uint8_t mkid = raw_kek["keks"][kid]["mkid"];
		BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);
		
		if (ui->encMode->isChecked()) {
			BIN outb = EAD7::enc(kek,conv::STRtoBIN(text),mkid,base::dec64(kid));
			out = base::enc64(outb);
		} else {
			BIN inputBin;
			try {
				inputBin = base::dec64(text);
			} catch (const exception) {
				u::stat("復号モードの入力がBase64URLSafe形式ではないので処理を中止しました");
				delm(raw_kek,kek);
				return;
			}
			BIN outb = EAD7::dec(kek,inputBin);
			out = conv::BINtoSTR(outb);
		}
		ui->out->setPlainText(QString::fromStdString(out));
		delm(raw_kek,kek,out);
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
		fb = new FileButton("配布KEKファイルを\n選択");
		QFont f = fb->font();
		f.setPointSize(72);
		f.setWeight(QFont::Bold);
		fb->setFont(f);
		fb->adjustSize();
		fb->show();
		QObject::connect(fb, &FileButton::fileSelected, [](const QString& qstr){mw::import_dst_kek(qstr,true);});
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
	ui->log->setVisible(false);
	w->show();
	
	windowSave::settingFile = SD+"GUI_setting.json";
	clipboard = QApplication::clipboard();
	
	ui->credit->setStyleSheet("color: tomato;");
	ui->credit->setText(QString::fromStdString("EAD7 " + ver.str() + " (C) 2025 yy981"));
	if (fs::exists(SDM)) {
		ui->adminMode->setEnabled(true);
		ui->adminMode->setChecked(true);
	}
	
	QObject::connect(app, &QCoreApplication::aboutToQuit, windowSave::save);
	QShortcut shortcut_inp_multi(QKeySequence("Alt+Return"), ui->inp_multi);
	QObject::connect(ui->run, &QPushButton::clicked, mw::run);
	QObject::connect(&shortcut_inp_multi, &QShortcut::activated, []{mw::setInpFrom(INP_FROM::multi); ui->run->animateClick();});
	QObject::connect(ui->inp_line, &QLineEdit::returnPressed, []{mw::setInpFrom(INP_FROM::line); ui->run->animateClick();});
	QObject::connect(ui->inp_file_path, &QLineEdit::returnPressed, []{mw::setInpFrom(INP_FROM::file); ui->run->animateClick();});
	QObject::connect(ui->inp_file_button, &FileButton::fileSelected, [](const QString& selectedFile){
		mw::setInpFrom(INP_FROM::file);
		ui->inp_file_path->setText(selectedFile);
		ui->run->animateClick();
	});
	QObject::connect(ui->log_checkbox, &QCheckBox::checkStateChanged, ui->log, &QPlainTextEdit::setVisible);
	QObject::connect(ui->dst_file, &FileButton::fileSelected, [](const QString& qstr){mw::import_dst_kek(qstr);});
	QObject::connect(ui->inp_from, &QComboBox::currentIndexChanged, [](const int& index){
		mw::inp_from = ui->inp_from->itemData(index).value<INP_FROM>();
	});
	QObject::connect(ui->copy, &QPushButton::clicked, []{clipboard->setText(ui->out->toPlainText());});
	QObject::connect(ui->clear, &QPushButton::clicked, ui->out, &QPlainTextEdit::clear);
	
	loadKeyCombobox();
	
	ui->inp_from->setItemData(0, QVariant::fromValue(INP_FROM::null));
	ui->inp_from->setItemData(1, QVariant::fromValue(INP_FROM::line));
	ui->inp_from->setItemData(2, QVariant::fromValue(INP_FROM::multi));
	ui->inp_from->setItemData(3, QVariant::fromValue(INP_FROM::file));
	
	if (fs::exists(windowSave::settingFile)) windowSave::load();

	u::log("ui setup完了");
	
	ui->inp_line->setFocus();
	
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
