#include <windows.h>
#include <wincred.h>
#include <tchar.h>

#include "token.h"


void saveToken(const BIN& token) {
	// 雉・ｼ諠・ｱ繧ｨ繝ｳ繝医Μ繧呈ｧ狗ｯ・
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC; 						// 豎守畑雉・ｼ諠・ｱ
	cred.TargetName = const_cast<LPWSTR>(L"yy981::EAD7_token");
	cred.CredentialBlobSize = token.size();
	cred.CredentialBlob = (LPBYTE)token.data();
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;			// 諱剃ｹ・噪縺ｫ菫晏ｭ・縺薙・繝ｦ繝ｼ繧ｶ繝ｼ蟆ら畑

	// 譖ｸ縺崎ｾｼ縺ｿ
	if (!CredWriteW(&cred, 0)) throw std::runtime_error("saveToken()::CredWrite failed: " + GetLastError());

}


BIN loadToken() {
	PCREDENTIALW pcred = nullptr;
	BIN restored;
	if (!CredReadW(L"yy981::EAD7_token", CRED_TYPE_GENERIC, 0, &pcred)) {
		DWORD err = GetLastError();
		if (err == ERROR_NOT_FOUND) {
			restored = randomBIN(32);
			saveToken(restored);
		} else throw std::runtime_error("loadToken()::CredRead failed: " + err);
	} else restored.Assign(pcred->CredentialBlob, pcred->CredentialBlobSize);
	
	CredFree(pcred);

	return restored;
}
