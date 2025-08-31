#include <windows.h>
#include <wincred.h>
#include <tchar.h>
#include <iostream>

#include "def.h"
#include "master.h"
#include "base.h"


void saveToken(const BIN& token) {
	// 資格情報エントリを構築
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC; 						// 汎用資格情報
	cred.TargetName = const_cast<LPWSTR>(L"yy981::EAD7_token");
	cred.CredentialBlobSize = token.size();
	cred.CredentialBlob = (LPBYTE)token.data();
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;			// 恒久的に保存 このユーザー専用

	// 書き込み
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
