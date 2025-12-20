#include <sstream>

#include "gui.h"
#include "../UI/util.h"
#include "../master.h"
#include "../base.h"
#include "../UI/info.h"

#include "mw.h"
#include "awv.h"


namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	
	void setInpFrom(const INP_FROM& inp_from_new) {
		inp_from = inp_from_new;
		
		int index = ui->inp_from->findData(QVariant::fromValue(inp_from));
		if (index != -1) ui->inp_from->setCurrentIndex(index);
	}
	
	void import_dst_kek(const QString& qstr, bool from_kek_window) {
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
	
	std::string textInfo(const std::string& text) {
		BIN bin = base::dec64(text);
		THEADER t;
		std::memcpy(&t, bin.data(),sizeof(THEADER));
		std::stringstream ss;
		ss << "[情報]\n"
		   << "ver: " << std::to_string(t.ver)
		   << "\nMK-ID: " << std::to_string(t.mkid)
		   << "\nKEK-ID: " << base::enc64(conv::ARRtoBIN(t.kid))
		   << "\nnonce: " << base::enc64(conv::ARRtoBIN(t.nonce));
		return ss.str();
	}

	void fileInfo(const std::string& path) {
		FDat f = getFileType(fs::path(path));
		std::string str = getFileInfo(true, f);
		ui->out->setPlainText(QString::fromStdString("[情報] - " + str));
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
			u::stat("暗号化処理完了");

		} else if (ui->decMode->isChecked()) {

			BIN inputBin;
			try {
				inputBin = base::dec64(text);
			} catch (const exception) {
				u::stat("復号モードの入力がBase64URLSafe形式ではないので処理を中止しました");
				delm(raw_kek);
				return;
			}
			
			if (inputBin.size() < 3 + 16) {
				u::stat("復号モードの入力が不正です（データ長不足）");
				delm(raw_kek);
				return;
			}

			BIN kidb(16);
			std::memcpy(kidb.data(), inputBin.data()+3, 16);
			std::string kid = base::enc64(kidb);

			// uint8_t mkid = inputBin[2];
			BIN kek;

			if (aui->OT_dec_enable->isChecked()) {
				kek = awv::OT_dec(base::dec64(kid));
			} else {
				if (!raw_kek["keks"].contains(kid)) {
					u::stat("指定されたKIDは鍵リストに存在しません");
					delm(raw_kek);
					return;
				}				
				kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			}

			std::string key_label = raw_kek["keks"][kid]["label"];

			BIN outb = EAD7::dec(kek,inputBin);
			out = conv::BINtoSTR(outb);

			u::stat("復号処理完了 使用した鍵: " + key_label);

		} else out = mw::textInfo(text);
		ui->out->setPlainText(QString::fromStdString(out));
		delm(raw_kek,out);
	}
	
	void fileProc(const std::string& path) {
		if (!fs::exists(path)) {
			u::stat("指定されたファイルが存在しません");
			return;
		}

		
		if (ui->encMode->isChecked()) {

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
			uint64_t chunkSize = ui->chunkSize->currentData().toULongLong();
			
			EAD7::encFile(kek,path,mkid,base::dec64(kid),chunkSize);
			delm(raw_kek);
			u::stat("ファイルの暗号化を正常に終了しました");

		} else if (ui->decMode->isChecked()) {

			FHeader fh = getFileHeader(path);
			BIN kid = conv::ARRtoBIN(fh.kid);

			json raw_kek = decPKEK(PKEK);
			BIN kek;

			if (aui->OT_dec_enable->isChecked()) {
				kek = awv::OT_dec(kid);
			} else {
				if (!raw_kek["keks"].contains(base::enc64(kid))) {
					u::stat("指定されたKIDは鍵リストに存在しません");
					delm(raw_kek);
					return;
				}				
				kek = base::dec64(raw_kek["keks"][base::enc64(kid)]["kek"]);
			}
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
			delm(raw_kek);

		} else mw::fileInfo(path);
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
