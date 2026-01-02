#include "info.h"

#include <string>
#include <sstream>

#include "../master.h"

#include "../CUI/ui.h"
#include "../GUI/gui.h"
#include "../UI/util.h"


std::string getFileInfo(bool isGUI, FDat& f) {
    bool isCUI = !isGUI;
    std::stringstream sout;

	switch (f.type) {
		case FSType::MK: {

            sout << "[MK]:マスターキーリスト\n存在するMKID: ";
			for (auto [key,value]: f.json.items()) {
				sout << key << ",";
			}
			sout << "\nより詳細な情報は管理者モードで起動して操作してください\n";

        } break;
		case FSType::kid: {

            sout << "[kid]:KIDリスト\nより詳細な情報は管理者モードで起動して操作してください\n";

        } break;
		case FSType::raw_kek: {

            sout << "[raw.kek]:生KEK"
					  << "\n作成日時:     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(f.json);

        } break;
		case FSType::p_kek: {

            json j = decPKEK(f.json);
			sout << "[p.kek]:通常KEK"
					  << "\n作成日時:     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(j);

        } break;
		case FSType::cus_kek: {

            // 後で実装するはず 未来の自分よろ
            // 知らんがな そもそもcus.kekの実装がまだや てことでさらに未来の自分よろ
			exit(1000);

        } break;
		case FSType::adm_kek: {

            sout << "[p.kek]:管理者KEK"
					  << "\n作成日時:     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}

        } break;
		case FSType::dst_kek: {

            sout << "[dst.kek]:配布KEK\n";
            std::string pass;
            if (isCUI) pass = inp_s("パスワード: ");
                else pass = prompt("パスワード: ");
			json j = decDstKEK(pass,f.json);
			sout << "\n作成日時:     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(pass, j);

        } break;
		case FSType::base64: {

            sout << "base64urlsafe\n";

        } break;
		case FSType::bin_e7: {

			sout << "[暗号化されたファイル]\n"
                 << "チャンク数: " << f.json["chunkNumber"] << "\n"
                 << "チャンクサイズ: " << formatBytes(f.json["chunkSize"]) << "\n"
                 << "最終チャンクサイズ: " << formatBytes(f.json["lastChunkSize"]) << "\n"
                 << "MK-ID: " << f.json["mkid"] << "\n"
                 << "KEK-ID: " << f.json["kid"] << "\n";

        } break;
			default: sout << "多分正しいe7系統データではない"; // 処理を実装してない部分だったらごめん
	}

    return sout.str();
}
