#include <iostream>
#include <fstream>

#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QInputDialog>

#include <yy981/string.h>

#include "cui.h"
#include "gui.h"
#include "../master.h"


std::vector<std::string> selectItem(const std::vector<Entry>& entries) {
	QDialog dialog;
	dialog.setWindowTitle("選択");

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QListWidget *listWidget = new QListWidget;

	CN(listWidget, &QListWidget::itemClicked, [&](QListWidgetItem *item) {
		if (item->checkState() == Qt::Checked) {
			item->setCheckState(Qt::Unchecked);
		} else {
			item->setCheckState(Qt::Checked);
		}
	});

	for (const auto &e : entries) {
		QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(e.label), listWidget);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
		item->setData(Qt::UserRole, QString::fromStdString(e.id));
	}
	layout->addWidget(listWidget);

	QPushButton *okButton = new QPushButton("OK");
	layout->addWidget(okButton);

	CN(okButton, &QPushButton::clicked, [&]() {
		dialog.accept();
	});

	std::vector<std::string> result;
	if (dialog.exec() == QDialog::Accepted) {
		for (int i = 0; i < listWidget->count(); ++i) {
			QListWidgetItem *item = listWidget->item(i);
			if (item->checkState() == Qt::Checked) {
				result.push_back(item->data(Qt::UserRole).toString().toStdString());
			}
		}
	} else std::runtime_error("selectItem():選択無し");
	return result;
}

std::string prompt(const std::string& placeholderText) {
	bool ok = false;
	QString text = QInputDialog::getText(
		nullptr,"入力",
		QString::fromStdString(placeholderText),
		QLineEdit::Normal,"",&ok
	);
	if (ok) return text.toStdString();
	return "";
}

json getKIDEntry(const std::string& label, const BIN& mk) {
	json result;
	for (const auto x: fs::recursive_directory_iterator(SDM)) {
		int number = -1;
		std::string pathString = x.path().stem().stem().string();
		try {
			number = std::stoi(pathString);
		} catch (...) {
			continue;
		}
		if (0 <= number && number <= 255) result = loadKID(mk,number);
	}
	return result;
}
