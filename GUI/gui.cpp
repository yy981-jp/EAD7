#include <fstream>

#include <yy981/env.h>

#include "../master.h"
#include "gui.h"
#include "../CUI/ui.h"
#include "../CUI/text.h"
#include "ui_main.h"

void u::setPrg(const int& v) {
    ui->progressBar->setValue(v);
}
void u::setPrgMax(const int& v) {
    ui->progressBar->setMaximum(v);
}
void u::log(const std::string& str) {
    ui->log->appendPlainText(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str));
}
void u::stat(const std::string& str) {
    ui->statusbar->showMessage(QString::fromStdString(convUnixTime(getUnixTime()) + ":     " + str), 60 * 1000);
}

void crashReport(const std::string& text) {
	std::string crash_log_path = getEnv("tmp")+"/EAD7_crash.log";
	std::ofstream(crash_log_path) << t::banner << "\n\n\n\n\n" << convUnixTime(getUnixTime())
								  << ":   " << text << "\n";
	openFile(crash_log_path);
}
