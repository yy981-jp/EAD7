#pragma once
#include <QtWidgets/QMainWindow>

#include "ui_main.h"
#include "../CUI/ui.h"


extern Ui::MainWindow* ui;
extern QMainWindow* w;

enum class INP_FROM {
	null, line, multi, file
};

namespace mw {
	extern INP_FROM inp_from;
}

namespace u {
	inline void setPrg(const int& v) {
		ui->progressBar->setValue(v);
	}
	inline void setPrgMax(const int& v) {
		ui->progressBar->setMaximum(v);
	}
	inline void log(const std::string& str) {
		ui->log->appendPlainText(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str));
	}
	inline void stat(const std::string& str) {
		ui->statusbar->showMessage(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str),60*1000); // 1分
	}
}
