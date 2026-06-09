#include <sstream>
#include <thread>
#include <atomic>

#include <QtCore/QTimer>

#include "gui.h"
#include "UI/util.h"
#include "master.h"
#include "base.h"
#include "UI/info.h"

#include "mw.h"
#include "awv.h"


namespace mw {
	INP_FROM inp_from = INP_FROM::null;
	std::jthread* thread;
	std::atomic<uint64_t> currentChunkNumber{0};
	std::vector<uint64_t> errorChunks;
	QTimer* progressTimer;
	uint64_t fileProcStartUnixTime = 0;
	bool fileProcessing = false;

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
						std::string pass = prompt("DST.KEK繝輔ぃ繧､繝ｫ縺ｮ繝代せ繝ｯ繝ｼ繝・ ");
						json raw_kek = decDstKEK(pass,f.json);
						json p_kek = encPKEK(raw_kek);
						writeJson(p_kek,path::p_kek);
						u::stat("P_KEK譖ｴ譁ｰ螳御ｺ・n");
						delm(pass,raw_kek);
						if (from_kek_window) fb->close();
						throw std::runtime_error("蜀崎ｵｷ蜍穂ｿ｡蜿ｷ(P_KEK蜀崎ｪｭ縺ｿ霎ｼ縺ｿ)");
					} break;
					default: throw std::runtime_error("E7繝輔ぃ繧､繝ｫ縺ｧ縺ｯ縺ゅｊ縺ｾ縺吶′縲∝ｽ｢蠑上′荳肴ｭ｣縺ｧ縺・);
				}
			}
		}
		u::stat("dst_kek縺ｨ縺励※蜈･蜉帙＆繧後◆繝輔ぃ繧､繝ｫ縺ｯe7繝輔ぃ繧､繝ｫ縺ｧ縺ｯ縺ゅｊ縺ｾ縺帙ｓ(繝輔ぃ繧､繝ｫ諡｡蠑ｵ蟄舌〒蛻､譁ｭ)");
	}
	
	std::string textInfo(const std::string& text) {
		BIN bin = base::dec64(text);
		THEADER t;
		std::memcpy(&t, bin.data(),sizeof(THEADER));
		std::stringstream ss;
		ss << "[諠・ｱ]\n"
		   << "ver: " << std::to_string(t.ver)
		   << "\nMK-ID: " << std::to_string(t.mkid)
		   << "\nKEK-ID: " << base::enc64(conv::ARRtoBIN(t.kid))
		   << "\nnonce: " << base::enc64(conv::ARRtoBIN(t.nonce));
		return ss.str();
	}

	void fileInfo(const std::string& path) {
		FDat f = getFileType(fs::path(path));
		std::string str = getFileInfo(true, f);
		ui->out->setPlainText(QString::fromStdString("[諠・ｱ] - " + str));
	}

	void textProc(const std::string& text) {
		if (text.empty()) {
			u::stat("蜈･蜉帙′縺ゅｊ縺ｾ縺帙ｓ");
			return;
		}
		std::string out;
		
		int index = ui->selectKey->currentIndex();
		json raw_kek = decPKEK(PKEK);
		
		if (ui->encMode->isChecked()) {

			std::string kid = ui->selectKey->itemData(index).toString().toStdString();
			uint8_t mkid = raw_kek["keks"][kid]["mkid"];
			if (kid.empty()) kid = ui->selectKey->currentText().toStdString(); // 邂｡逅・・Δ繝ｼ繝画焔蜍募・蜉帛ｯｾ蠢・
			if (kid.empty()) {
				u::stat("菴ｿ逕ｨ縺吶ｋ骰ｵ繧帝∈謚槭＠縺ｦ縺上□縺輔＞");
				return;
			}
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			BIN outb = EAD7::enc(kek,conv::STRtoBIN(text),mkid,base::dec64(kid));
			out = base::enc64(outb);
			u::stat("證怜捷蛹門・逅・ｮ御ｺ・);

		} else if (ui->decMode->isChecked()) {

			BIN inputBin;
			try {
				inputBin = base::dec64(text);
			} catch (const exception) {
				u::stat("蠕ｩ蜿ｷ繝｢繝ｼ繝峨・蜈･蜉帙′Base64URLSafe蠖｢蠑上〒縺ｯ縺ｪ縺・・縺ｧ蜃ｦ逅・ｒ荳ｭ豁｢縺励∪縺励◆");
				delm(raw_kek);
				return;
			}
			
			if (inputBin.size() < 3 + 16) {
				u::stat("蠕ｩ蜿ｷ繝｢繝ｼ繝峨・蜈･蜉帙′荳肴ｭ｣縺ｧ縺呻ｼ医ョ繝ｼ繧ｿ髟ｷ荳崎ｶｳ・・);
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
					u::stat("謖・ｮ壹＆繧後◆KID縺ｯ骰ｵ繝ｪ繧ｹ繝医↓蟄伜惠縺励∪縺帙ｓ");
					delm(raw_kek);
					return;
				}				
				kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			}

			std::string key_label = raw_kek["keks"][kid]["label"];

			BIN outb = EAD7::dec(kek,inputBin);
			out = conv::BINtoSTR(outb);

			u::stat("蠕ｩ蜿ｷ蜃ｦ逅・ｮ御ｺ・菴ｿ逕ｨ縺励◆骰ｵ: " + key_label);

		} else out = mw::textInfo(text);
		ui->out->setPlainText(QString::fromStdString(out));
		delm(raw_kek,out);
	}
	
	void fileProc(const std::string& path) {
		if (!fs::exists(path)) {
			u::stat("謖・ｮ壹＆繧後◆繝輔ぃ繧､繝ｫ縺悟ｭ伜惠縺励∪縺帙ｓ");
			return;
		}

		
		if (ui->encMode->isChecked()) {
			if (fileProcessing) {
				u::stat("繝輔ぃ繧､繝ｫ繧貞・逅・ｸｭ縺ｧ縺・);
				return;
			}
			fileProcessing = true;
			fileProcStartUnixTime = getUnixTime();
			// cleanup global
			if (progressTimer) {
				progressTimer->stop();
				progressTimer->deleteLater();
				progressTimer = nullptr;
			}
			if (thread) {
				try {
					thread->request_stop();
				} catch (...) {}
				delete thread;
				thread = nullptr;
			}
			currentChunkNumber.store(0);
			ui->progressBar->setValue(0);

			int index = ui->selectKey->currentIndex();
			std::string kid = ui->selectKey->itemData(index).toString().toStdString();
			if (kid.empty()) kid = ui->selectKey->currentText().toStdString(); // 邂｡逅・・Δ繝ｼ繝画焔蜍募・蜉帛ｯｾ蠢・
			if (kid.empty()) {
				u::stat("菴ｿ逕ｨ縺吶ｋ骰ｵ繧帝∈謚槭＠縺ｦ縺上□縺輔＞");
				return;
			}

			json raw_kek = decPKEK(PKEK);
			uint8_t mkid = raw_kek["keks"][kid]["mkid"];
			BIN kek = base::dec64(raw_kek["keks"][kid]["kek"]);
			uint64_t chunkSize = ui->chunkSize->currentData().toULongLong();

			uint64_t totalChunkNumber = (fs::file_size(path) / chunkSize) + 1;
			thread = new std::jthread(EAD7::encFile,kek,path,mkid,base::dec64(kid),chunkSize,&currentChunkNumber);
			
			progressTimer = new QTimer;
			ui->progressBar->setRange(0, static_cast<int>(totalChunkNumber));
			CN(progressTimer, &QTimer::timeout, [totalChunkNumber]{
				ui->progressBar->setValue(static_cast<int>(mw::currentChunkNumber.load()));
				if (mw::currentChunkNumber.load() >= totalChunkNumber) {
					uint64_t procTime = getUnixTime() - fileProcStartUnixTime;
					u::stat("繝輔ぃ繧､繝ｫ縺ｮ證怜捷蛹悶ｒ豁｣蟶ｸ縺ｫ邨ゆｺ・＠縺ｾ縺励◆ (" + formatSeconds(procTime) + ")");
					progressTimer->stop();
					fileProcessing = false;
				}
			});
			progressTimer->start(50); // 20fps

			delm(raw_kek);

		} else if (ui->decMode->isChecked()) {
			if (fileProcessing) {
				u::stat("繝輔ぃ繧､繝ｫ繧貞・逅・ｸｭ縺ｧ縺・);
				return;
			}
			fileProcessing = true;
			fileProcStartUnixTime = getUnixTime();

			// cleanup global
			if (progressTimer) {
				progressTimer->stop();
				progressTimer->deleteLater();
				progressTimer = nullptr;
			}
			if (thread) {
				try {
					thread->request_stop();
				} catch (...) {}
				delete thread;
				thread = nullptr;
			}
			currentChunkNumber.store(0);
			errorChunks.clear();
			ui->progressBar->setValue(0);


			FHeader fh = getFileHeader(path);
			BIN kid = conv::ARRtoBIN(fh.kid);
			uint64_t totalChunkNumber = fh.chunkNumber;

			json raw_kek = decPKEK(PKEK);
			BIN kek;

			if (aui->OT_dec_enable->isChecked()) {
				kek = awv::OT_dec(kid);
			} else {
				if (!raw_kek["keks"].contains(base::enc64(kid))) {
					u::stat("謖・ｮ壹＆繧後◆KID縺ｯ骰ｵ繝ｪ繧ｹ繝医↓蟄伜惠縺励∪縺帙ｓ");
					delm(raw_kek);
					return;
				}				
				kek = base::dec64(raw_kek["keks"][base::enc64(kid)]["kek"]);
			}
			
			thread = new std::jthread([=]{
				errorChunks = EAD7::decFile(kek,path,&currentChunkNumber);
			});

			progressTimer = new QTimer;
			ui->progressBar->setRange(0, static_cast<int>(totalChunkNumber));
			CN(progressTimer, &QTimer::timeout, [totalChunkNumber]{
				ui->progressBar->setValue(static_cast<int>(mw::currentChunkNumber.load()));
				if (mw::currentChunkNumber.load() >= totalChunkNumber) {
					uint64_t procTime = getUnixTime() - fileProcStartUnixTime;
					if (errorChunks.empty()) {
						u::stat("繝輔ぃ繧､繝ｫ縺ｮ蠕ｩ蜿ｷ繧呈ｭ｣蟶ｸ縺ｫ邨ゆｺ・＠縺ｾ縺励◆ (" + formatSeconds(procTime) + ")");
					} else {
						u::stat("繝輔ぃ繧､繝ｫ縺ｮ蠕ｩ蜿ｷ荳ｭ縺ｫ繧ｨ繝ｩ繝ｼ縺檎匱逕溘＠縺ｾ縺励◆ 隧ｳ邏ｰ縺ｯ繝ｭ繧ｰ繧堤｢ｺ隱阪＠縺ｦ縺上□縺輔＞ (" + formatSeconds(procTime) + ")");
						std::string result;
						for (const uint64_t& cn: errorChunks) result += std::to_string(cn) + ",";
						result.pop_back();
						u::log("遐ｴ謳阪＠縺溘メ繝｣繝ｳ繧ｯ逡ｪ蜿ｷ: " + result);
					}
					progressTimer->stop();
					fileProcessing = false;
				}
			});
			progressTimer->start(50); // 20fps
			
			delm(raw_kek);
		} else mw::fileInfo(path);
	}
	
	void run() {
		std::string text;
		switch (inp_from) {
			case INP_FROM::null: u::stat("蜈･蜉帛・縺檎音螳壹〒縺阪∪縺帙ｓ"); return;
			case INP_FROM::line: text = ui->inp_line->text().toStdString(); break;
			case INP_FROM::multi: text = ui->inp_multi->toPlainText().toStdString(); break;
			case INP_FROM::file: text = ui->inp_file_path->text().toStdString(); break;
		}
		if (inp_from == INP_FROM::file) fileProc(text); else textProc(text);
	}

}
