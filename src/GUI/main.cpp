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
#include "widgets/mainWindow.h"

namespace awv {
	void MK_load() {
		const std::vector<QComboBox*> comboBoxes = {
			aui->MK_unWrap_index,
			aui->KID_create_index,
			aui->KID_recal_index,
			aui->KEK_index,
			aui->OT_dst_index
		};

		json MK = readJson(path::MK);
		QStringList mkids;
		for (const auto& [index,object]: MK.items()) {
			mkids << QString::fromStdString(index);
		}
		
		for (QComboBox* c: comboBoxes) {
			c->clear();
			c->addItems(mkids);
		}
		
	}
	
	void MK_clear() {
		aui->MK_unWrap_pass->clear();
		aui->MK_unWrap_out->clear();
		aui->MK_create_pass->clear();
		aui->MK_create_b64->clear();
	}


	void MK_unWrap() {
		QString mkid_qs = aui->MK_unWrap_index->currentText();
		std::string mkpass = aui->MK_unWrap_pass->text().toStdString();
		if (mkid_qs.isEmpty() || mkpass.empty()) {u::stat("MK_unWrap: 入力が不足しています");return;}
		uint8_t mkid = mkid_qs.toInt();
		QString out_qs = QString::fromStdString(base::enc64(loadMK(mkid,mkpass)));
		aui->MK_unWrap_out->setText(out_qs);
		delm(mkpass);
	}
	
	void MK_create() {
		uint8_t mkid = aui->MK_create_index->value();
		std::string mkpass = aui->MK_create_pass->text().toStdString();
		if (mkpass.empty()) {
			u::stat("MK_create: 入力が不足しています");
			delm(mkpass);
			return;
		}
		
		std::string b64 = aui->MK_create_b64->text().toStdString();
		if (b64.empty()) {
			createMK(mkid,mkpass);
		} else {
			BIN b64_bin;
			try {
				b64_bin = base::dec64(b64);
			} catch (...) {
				u::stat("MK_create: Base64URLSafeの形式が不正です");
				delm(mkpass);
				return;
			}
			createMK(mkid,mkpass,b64_bin);
		}
		u::sl("MK_create: 完了");
		delm(mkpass);
	}
	
	void KID_create() {
		QString mkid_qs = aui->KID_create_index->currentText();
		std::string mkpass = aui->KID_create_mkpass->text().toStdString();
		KIDEntry entry;
		entry.label = aui->KID_create_label->text().toStdString();
		entry.b64 = aui->KID_create_b64->text().toStdString();
		entry.note = aui->KID_create_note->toPlainText().toStdString();
		entry.status = KStat::active;
		
		if (mkid_qs.isEmpty() || mkpass.empty() || entry.label.empty()) {
			u::stat("KID_create: 入力が不足しています");
			return;
		}
		uint8_t mkid = mkid_qs.toInt();
		BIN mk = loadMK(mkid,mkpass);
		ordered_json j = loadKID(mk,mkid);
		if (j.contains(entry.label)) {
			// 中断
		}
		if (!isBase64UrlSafe(entry.b64)) {
			u::stat("KID_create: Base64URLSafeの形式が不正です");
			return;
		}
		
		addNewKid(j,entry);
		saveKID(mk,mkid,j);
		
		u::sl("KID_create: 完了");
		delm(mk,mkpass);
	}
	
	void KID_recal() {
		QString mkid_qs = aui->KID_recal_index->currentText();
		std::string mkpass = aui->KID_recal_mkpass->text().toStdString();
		if (mkid_qs.isEmpty() || mkpass.empty()) {
			u::stat("KID_recal: 入力が不足しています");
			return;
		}
		uint8_t mkid = mkid_qs.toInt();
		BIN mk = loadMK(mkid,mkpass);
		
		json j = readJson(SDM+std::to_string(mkid)+".kid.e7").at("body");
		saveKID(mk,mkid,j);
		
		u::sl("KID_recal: 完了");
		delm(mk,mkpass);
	}
}

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
		if (text.empty()) {
			u::stat("入力がありません");
			return;
		}
		std::string out;
		
		int index = ui->selectKey->currentIndex();
		json raw_kek = decPKEK(PKEK);
		
		if (ui->encMode->isChecked()) {

			std::string kid = ui->selectKey->itemData(index).toString().toStdString();
			uint8_t mkid = raw_kek["keks"][kid]["mkid"];
			if (kid.empty()) kid = ui->selectKey->currentText().toStdString(); // 管理者モード手動入力対応
			if (kid.empty()) {
				u::stat("使用する鍵を選択してください");
				return;
			}
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			BIN outb = EAD7::enc(kek,conv::STRtoBIN(text),mkid,base::dec64(kid));
			out = base::enc64(outb);
			delm(kek);
		} else {
			BIN inputBin;
			try {
				inputBin = base::dec64(text);
			} catch (const exception) {
				u::stat("復号モードの入力がBase64URLSafe形式ではないので処理を中止しました");
				delm(raw_kek);
				return;
			}
			
			BIN kidb;
			std::memcpy(kidb.data(), inputBin.data()+3, 16);
			std::string kid = base::enc64(kidb);
			uint8_t mkid = raw_kek["keks"][kid]["mkid"];

			if (!raw_kek["keks"].contains(kid)) {
				u::stat("指定されたKIDは鍵リストに存在しません");
				delm(raw_kek);
				return;
			}
				
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);

			BIN outb = EAD7::dec(kek,inputBin);
			out = conv::BINtoSTR(outb);
		}
		ui->out->setPlainText(QString::fromStdString(out));
		delm(raw_kek,out);
	}
	
	void fileProc(const std::string& path) {
		if (!fs::exists(path)) {
			u::stat("指定されたファイルが存在しません");
			return;
		}

		int index = ui->selectKey->currentIndex();
		std::string kid = ui->selectKey->itemData(index).toString().toStdString();
		if (kid.empty()) kid = ui->selectKey->currentText().toStdString(); // 管理者モード手動入力対応
		if (kid.empty()) {
			u::stat("使用する鍵を選択してください");
			return;
		}
		
		if (ui->encMode->isChecked()) {

			json raw_kek = decPKEK(PKEK);
			uint8_t mkid = raw_kek["keks"][kid]["mkid"];
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			uint64_t chunkSize = ui->chunkSize->currentData().toULongLong();
			
			EAD7::encFile(kek,path,mkid,base::dec64(kid),chunkSize);
			delm(raw_kek,kek);
			u::stat("ファイルの暗号化を正常に終了しました");

		} else {

			json raw_kek = decPKEK(PKEK);
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);

			std::vector<uint64_t> outv = EAD7::decFile(kek,path);
			if (outv.empty()) {
				u::stat("ファイルの復号を正常に終了しました");
			} else {
				u::stat("ファイルの復号中にエラーが発生しました 詳細はログを確認してください");
				bool headerERR = false;
				for (const uint64_t& cn: outv) {
					if (cn == 0) {
						headerERR = true;
						continue;
					}
					u::log("破損したチャンク番号: " + std::to_string(cn));
				}
				if (headerERR) u::log("ファイルヘッダーの復号に失敗しました",true);
			}
			delm(raw_kek,kek);
		}
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
	CN(ui->copy, &QPushButton::clicked, []{clipboard->setText(ui->out->toPlainText());});
	CN(ui->clear, &QPushButton::clicked, ui->out, &QPlainTextEdit::clear);
	
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
