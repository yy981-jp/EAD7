#include "info.h"

#include <string>
#include <sstream>

#include "master.h"

#include "CUI/ui.h"
#include "../GUI/gui.h"
#include "UI/util.h"


std::string getFileInfo(bool isGUI, FDat& f) {
    bool isCUI = !isGUI;
    std::stringstream sout;

	switch (f.type) {
		case FSType::MK: {

            sout << "[MK]:マスターキーリスチEn存在するMKID: ";
			for (auto [key,value]: f.json.items()) {
				sout << key << ",";
			}
			sout << "\nより詳細な惁E��は管琁E��E��ードで起動して操作してください\n";

        } break;
		case FSType::kid: {

            sout << "[kid]:KIDリスチEnより詳細な惁E��は管琁E��E��ードで起動して操作してください\n";

        } break;
		case FSType::raw_kek: {

            sout << "[raw.kek]:生KEK"
					  << "\n作�E日晁E     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日晁E " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状慁E " << v["status"]
						  << "\n\t\t作�E日晁E " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(f.json);

        } break;
		case FSType::p_kek: {

            json j = decPKEK(f.json);
			sout << "[p.kek]:通常KEK"
					  << "\n作�E日晁E     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日晁E " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状慁E " << v["status"]
						  << "\n\t\t作�E日晁E " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(j);

        } break;
		case FSType::cus_kek: {

            // 後で実裁E��る�EぁE未来の自刁E��めE
            // 知らんがな そもそもcus.kekの実裁E��まだめEてことでさらに未来の自刁E��めE
			exit(1000);

        } break;
		case FSType::adm_kek: {

            sout << "[p.kek]:管琁E��EEK"
					  << "\n作�E日晁E     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日晁E " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状慁E " << v["status"]
						  << "\n\t\t作�E日晁E " << convUnixTime(v["created"])
						  << "\n\t}";
			}

        } break;
		case FSType::dst_kek: {

            sout << "[dst.kek]:配布KEK\n";
            std::string pass;
            if (isCUI) pass = inp_s("パスワーチE ");
                else pass = prompt("パスワーチE ");
			json j = decDstKEK(pass,f.json);
			sout << "\n作�E日晁E     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日晁E " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				sout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状慁E " << v["status"]
						  << "\n\t\t作�E日晁E " << convUnixTime(v["created"])
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
			default: sout << "多�E正しいe7系統チE�EタではなぁE; // 処琁E��実裁E��てなぁE��刁E��ったらごめめE
	}

    return sout.str();
}
