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
#include "../version.h"

#include "mw.h"
#include "awv.h"

#include "widgets/fileButton.h"
#include "widgets/mainWindow.h"


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
		CN(fb, &FileButton::fileSelected, [](const QString& qstr){mw::import_dst_kek(qstr,true);});
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
		// ui->adminMode->setEnabled(true);
		ui->adminMode->setChecked(true);
	}
	ui->inp_line->setEnabled(false);
	
	
	CN(app, &QCoreApplication::aboutToQuit, windowSave::save);
	shortcut_inp_multi = new QShortcut(QKeySequence("Alt+Return"), ui->inp_multi);
	CN(ui->run, &QPushButton::clicked, mw::run);
	CN(shortcut_inp_multi, &QShortcut::activated, []{u::stat("D: inp_multi"); mw::setInpFrom(INP_FROM::multi); ui->run->animateClick();});
	CN(ui->inp_line, &QLineEdit::returnPressed, []{mw::setInpFrom(INP_FROM::line); ui->run->animateClick();});
	CN(ui->inp_file_path, &QLineEdit::returnPressed, []{mw::setInpFrom(INP_FROM::file); ui->run->animateClick();});
	CN(ui->inp_file_button, &FileButton::fileSelected, [](const QString& selectedFile){
		mw::setInpFrom(INP_FROM::file);
		ui->inp_file_path->setText(selectedFile);
		ui->run->animateClick();
	});
	
	CN(ui->encMode, &QRadioButton::toggled, [](const bool encMode){
		ui->inp_line->setEnabled(!encMode);
		ui->inp_multi->setEnabled(encMode);
	});
	
	CN(ui->log_checkbox, &QCheckBox::checkStateChanged, ui->log, &QPlainTextEdit::setVisible);
	CN(ui->dst_file, &FileButton::fileSelected, [](const QString& qstr){mw::import_dst_kek(qstr);});
	CN(ui->inp_from, &QComboBox::currentIndexChanged, [](const int& index){
		mw::inp_from = ui->inp_from->itemData(index).value<INP_FROM>();
	});
	CN(ui->copy, &QPushButton::clicked, []{
		QString qstring = ui->out->toPlainText();
		if (qstring.isEmpty()) {
			u::stat("出力内容が空です");
			return;
		}
		clipboard->setText(qstring);
		u::stat("出力内容をクリップボードにコピーしました");
	});
	CN(ui->clear, &QPushButton::clicked, ui->out, &QPlainTextEdit::clear);

	// shortcuts
	CN(new QShortcut(QKeySequence("Ctrl+C"), w), &QShortcut::activated, ui->copy, &QPushButton::animateClick);
	CN(new QShortcut(QKeySequence("Ctrl+X"), w), &QShortcut::activated, ui->clear, &QPushButton::animateClick);
	CN(new QShortcut(QKeySequence("Ctrl+O"), w), &QShortcut::activated, ui->inp_file_button, &QPushButton::animateClick);
	CN(new QShortcut(QKeySequence("Ctrl+E"), w), &QShortcut::activated, [&]{ui->encMode->setChecked(true);});
	CN(new QShortcut(QKeySequence("Ctrl+D"), w), &QShortcut::activated, [&]{ui->decMode->setChecked(true);});
	CN(new QShortcut(QKeySequence("ESC"), w), &QShortcut::activated, [&]{w->setFocus();});
	CN(new QShortcut(QKeySequence("Ctrl+I"), w), &QShortcut::activated, [&]{
		if (ui->inp_line->isEnabled()) ui->inp_line->setFocus();
			else ui->inp_multi->setFocus();
	});
	
	loadKeyCombobox();
	
	// ui.inp_from
	ui->inp_from->setItemData(0, QVariant::fromValue(INP_FROM::null));
	ui->inp_from->setItemData(1, QVariant::fromValue(INP_FROM::line));
	ui->inp_from->setItemData(2, QVariant::fromValue(INP_FROM::multi));
	ui->inp_from->setItemData(3, QVariant::fromValue(INP_FROM::file));

	// ui.chunkSize
	ui->chunkSize->addItem("4 KiB",		QVariant(4ULL * 1024));
	ui->chunkSize->addItem("8 KiB",		QVariant(8ULL * 1024));
	ui->chunkSize->addItem("16 KiB",	QVariant(16ULL * 1024));
	ui->chunkSize->addItem("32 KiB",	QVariant(32ULL * 1024));
	ui->chunkSize->addItem("64 KiB",	QVariant(64ULL * 1024));
	ui->chunkSize->addItem("128 KiB",	QVariant(128ULL * 1024));
	ui->chunkSize->addItem("256 KiB",	QVariant(256ULL * 1024));
	ui->chunkSize->addItem("512 KiB",	QVariant(512ULL * 1024));
	ui->chunkSize->addItem("1 MiB",		QVariant(1ULL * 1024 * 1024));
	ui->chunkSize->addItem("2 MiB",		QVariant(2ULL * 1024 * 1024));
	ui->chunkSize->addItem("4 MiB",		QVariant(4ULL * 1024 * 1024));
	ui->chunkSize->addItem("8 MiB",		QVariant(8ULL * 1024 * 1024));
	ui->chunkSize->addItem("16 MiB",	QVariant(16ULL * 1024 * 1024));

	int idx = ui->chunkSize->findData(QVariant(CHUNKSIZE::default_size));
	ui->chunkSize->setCurrentIndex(idx);
	
	// admin ui
	
	// ### AdminUI MK
	CN(aui->MK_load, &QPushButton::clicked, awv::MK_load);
	CN(aui->MK_clear, &QPushButton::clicked, awv::MK_clear);
	CN(aui->MK_open, &QPushButton::clicked, []{openFile(path::MK);});
	CN(aui->MK_unWrap_pass, &QLineEdit::returnPressed, awv::MK_unWrap);
	CN(aui->MK_create_pass, &QLineEdit::returnPressed, awv::MK_create);
	
	// ### AdminUI KID
	CN(aui->KID_open, &QPushButton::clicked, []{openFile(SDM);});
	CN(aui->KID_create_run, &QPushButton::clicked, awv::KID_create);
	
	
	awv::MK_load();
	// restore window
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
			delete w;	w = nullptr;
			delete ui;	ui = nullptr;
			w = new MainWindow;
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
			delete w; // QThreadStorageエラー対策 (できてない)
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
