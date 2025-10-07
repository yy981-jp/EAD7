#pragma once
#include <QtWidgets/QMainWindow>

#include "ui_main.h"
#include "../CUI/ui.h"


extern Ui::MainWindow* ui;
extern QMainWindow* w;


namespace u {
	void setPrg(const int& v) {
		ui->progressBar->setValue(v);
	}
	void log(const std::string& str) {
		ui->log->appendPlainText(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str));
	}
	void stat(const std::string& str) {
		ui->statusbar->showMessage(QString::fromStdString(str),60*1000); // 1分
	}
}
