#include <QtGui/QStandardItemModel>

#include "gui.h"
#include "../master.h"
#include "../base.h"
#include "widgets/twoTreeView.h"
#include "../UI/util.h"

#include "awv.h"

#include <iostream>

namespace awv {
	std::vector<uint8_t> MK_load() {
		std::vector<uint8_t> result;
		static const std::vector<QComboBox*> comboBoxes = {
			aui->MK_unWrap_index,
			aui->KID_create_index,
			aui->KID_recal_index,
			aui->KEK_index,
			aui->OT_dec_mk_index
		};

		json MK = readJson(path::MK);
		QStringList mkids;
		for (const auto& [index,object]: MK.items()) {
			result.emplace_back(std::stoi(index));
			mkids << QString::fromStdString(index);
		}
		
		for (QComboBox* c: comboBoxes) {
			c->clear();
			c->addItems(mkids);
		}
		
		return result;
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
		
		json j = readJson(getKIDFilePath(mkid)).at("body");
		saveKID(mk,mkid,j);
		
		u::sl("KID_recal: 完了");
		delm(mkpass);
	}

	void KEK_KIDLoad() {
		std::vector<uint8_t> mkids = MK_load();
		std::map<std::string,std::vector<std::string>> items;
		for (const uint8_t mkid: mkids) {
			std::vector<std::string> items_child;
			std::string kidfpath = getKIDFilePath(mkid);
			if (!fs::exists(kidfpath)) continue;
			json kids = readJson(kidfpath).at("body").at("kids");
			for (auto [label,entry]: kids.items()) {
				items_child.emplace_back(label);
			}
			items[std::string{"MK-ID: "} + std::to_string(mkid)] = items_child;
		}

		std::vector<std::string> nullData;

		aui->KEK_leftTree->init(true,TwoTreeView::convModel("KIDAllList", items));
		aui->KEK_rightTree->init(false,TwoTreeView::convModel("rihtPanel", nullData));
	}

	void KEK_write() {
		std::string mkid_s = aui->KEK_index->currentText().toStdString();
		std::string mkpass = aui->KEK_MKpass->text().toStdString();
		std::string target = aui->KEK_target->currentText().toStdString();
		if (mkid_s.empty() || mkpass.empty()) { u::stat("KEK_write: 入力が不足しています"); return; }
		if (target.empty()) { u::stat("KEK_write: 保存先ファイル名を入力してください"); return; }

		uint8_t mkid = std::stoi(mkid_s);
		BIN mk = loadMK(mkid, mkpass);

		// 選択されたラベル一覧を右ペインから取得
		std::vector<std::string> selected = aui->KEK_rightTree->getFlatModel();
		if (selected.empty()) { u::stat("KEK_write: 選択されたKIDがありません"); delm(mkpass); return; }

		// KIDファイルを読み、ラベルに対応するエントリを抽出
		json kid_body = loadKID(mk, mkid);
		json kids_selected = json::object();

		// std::cout << "D::body: " << kid_body.dump(4) << "\n\n";

		for (const auto& label : selected) {
			bool found = false;
			for (auto &it : kid_body["kids"].items()) {
				const std::string kid_b64 = it.key();
				const json entry = it.value();
				if (kid_b64 == label) {
					kids_selected[kid_b64] = entry;
					found = true;
					break;
				}
			}
			if (!found) u::stat(std::string("KEK_write: KIDが見つかりません: ") + label);
		}

		if (kids_selected.empty()) { u::stat("KEK_write: 有効なKIDが選択されていません"); delm(mkpass); return; }

		// RAW と ADM 形式に変換して保存
		json raw_kek = createRawKEK(mk, json::object(), kids_selected, mkid);
		json adm_kek = encAdmKEK(mk, raw_kek, mkid);
		writeJson(adm_kek, getAdmKEKPath(target));

		u::sl("KEK_write: 完了");
		delm(mkpass, raw_kek);
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


	void OT_DST() {
		std::string target = aui->OT_dst_index->text().toStdString();
		std::string mkpass = aui->OT_dst_mkpass->text().toStdString();
		std::string dst_pass = aui->OT_dst_pass->text().toStdString();
		if (target.empty() || mkpass.empty() || dst_pass.empty()) {
			u::stat("OT_DST: 入力が不足しています");
			return;
		}

		if (target.ends_with(".adm.kek.e7")) {
			target = target.substr(0, target.size() - std::string(".adm.kek.e7").size());
		}

		std::string adm_path = getAdmKEKPath(target);
		if (!fs::exists(adm_path)) {
			u::stat("OT_DST: ADM.KEKファイルが見つかりません");
			return;
		}

		json adm_kek;
		try {
			adm_kek = readJson(adm_path);
		} catch (const std::exception& e) {
			u::stat(std::string("OT_DST: ADM.KEK読み込み失敗: ") + e.what());
			return;
		}

		if (!adm_kek.contains("meta") || !adm_kek["meta"].contains("mkid")) {
			u::stat("OT_DST: ADM.KEK形式が不正です");
			delm(mkpass, dst_pass, adm_kek);
			return;
		}

		uint8_t mkid = adm_kek["meta"]["mkid"].get<uint8_t>();
		BIN mk;
		try {
			mk = loadMK(mkid, mkpass);
		} catch (const std::exception& e) {
			u::stat(std::string("OT_DST: MK読み込み失敗: ") + e.what());
			delm(mkpass, dst_pass, adm_kek);
			return;
		}

		json raw_kek;
		try {
			raw_kek = decAdmKEK(mk, adm_kek);
		} catch (const std::exception& e) {
			u::stat(std::string("OT_DST: ADM.KEK復号失敗: ") + e.what());
			delm(mkpass, dst_pass, adm_kek);
			return;
		}

		json dst_kek;
		try {
			dst_kek = encDstKEK(dst_pass, raw_kek);
		} catch (const std::exception& e) {
			u::stat(std::string("OT_DST: DST変換失敗: ") + e.what());
			delm(mkpass, dst_pass, adm_kek, raw_kek);
			return;
		}

		fs::path out_path = fs::current_path() / target;
		writeJson(dst_kek, out_path.string() + ".dst.kek.e7");
		u::sl("OT_DST: 完了");
		delm(mkpass, dst_pass, adm_kek, raw_kek);
	}
}
