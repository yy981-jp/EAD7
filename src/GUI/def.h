#pragma once
#include "../def.h"
#include "ui_main.h"
#include "ui_admin.h"
#include "mainWindow.h"


template <typename... Args>
void CN(Args&&... args) {
	QObject::connect(std::forward<Args>(args)...);
}

class MainWindow;
extern Ui::MainWindow* ui;
extern Ui::AdminWindow* aui;
extern MainWindow* w;
extern FileButton* fb;
extern QClipboard* clipboard;
extern QShortcut* shortcut_inp_multi;
extern json PKEK;

struct THEADER {
	uint8_t magic, ver, mkid;
	std::array<uint8_t,16> kid;
	std::array<uint8_t,12> nonce;
};
