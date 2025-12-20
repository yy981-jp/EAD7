#include "gui.h"
#include "../master.h"
#include "../base.h"
#include "../UI/util.h"

#include "awv.h"

namespace awv {
	void MK_load() {
		const std::vector<QComboBox*> comboBoxes = {
			aui->MK_unWrap_index,
			aui->KID_create_index,
			aui->KID_recal_index,
			aui->KEK_index,
			aui->OT_dst_index,
			aui->OT_dec_mk_index
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
	
	void KID_create_write() {
		QString mkid_qs = aui->KID_create_index->currentText();
		std::string mkpass = aui->KID_create_mkpass->text().toStdString();
		KIDEntry entry;
		entry.label = aui->KID_create_label->text().toStdString();
		entry.b64 = aui->KID_create_b64->text().toStdString();
		entry.note = aui->KID_create_note->toPlainText().toStdString();
		entry.status = KStat::active;
		
		if (entry.b64.empty()) entry.b64 = base::enc64(randomBIN(16));
		
		if (mkid_qs.isEmpty() || mkpass.empty() || entry.label.empty()) {
			u::stat("KID_create_write: 入力が不足しています");
			return;
		}
		uint8_t mkid = mkid_qs.toInt();
		BIN mk = loadMK(mkid,mkpass);
		json j = loadKID(mk,mkid);
		if (!isBase64UrlSafe(entry.b64)) {
			u::stat("KID_create_write: Base64URLSafeの形式が不正です");
			return;
		}

		std::pair<std::string,ordered_json> entry_j = makeKidEntry(entry);
		j["kids"][entry_j.first] = entry_j.second;

		saveKID(mk,mkid,j);
		
		u::sl("KID_create_write: 完了");
		delm(mkpass);
	}

	void KID_create_load() {
		QString mkid_qs = aui->KID_create_index->currentText();
		std::string mkpass = aui->KID_create_mkpass->text().toStdString();
		uint8_t mkid = mkid_qs.toInt();
		BIN mk = loadMK(mkid,mkpass);
		if (aui->KID_create_label->text().isEmpty()) {
			u::stat("KID_create_load: 入力が不足しています");
			return;
		} 
		ordered_json j = loadKID(mk,mkid);
		
		KIDEntry entry;
		
		entry.label = aui->KID_create_label->text().toStdString();

		if (!j.at("keks").contains(entry.label)) {
			u::stat("KID_create_load: 対象のエントリが見つかりません");
			return;
		}
		
		json entry_j = j["keks"][entry.label];
		aui->KID_create_b64->setText(QString::fromStdString(entry_j["b64"]));
		aui->KID_create_note->setPlainText(QString::fromStdString(entry_j["note"]));
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
		delm(mkpass);
	}

	/// @param kid 
	/// @return kek 
	BIN OT_dec(BIN kid) {
		QString mkid_qs = aui->OT_dec_mk_index->currentText();
		std::string mkpass = aui->OT_dec_mk_pass->text().toStdString();
		if (mkid_qs.isEmpty() || mkpass.empty()) throw std::runtime_error("OT_dec: 入力が不足しています");
		uint8_t mkid = mkid_qs.toInt();
		BIN mk = loadMK(mkid,mkpass);
		return deriveKEK(mk, base::enc64(kid));
	}
}
