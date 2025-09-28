#include "ui.h"
#include <iostream>

#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>


std::vector<std::string> selectItem(const std::vector<Entry>& entries) {
	QDialog dialog;
	dialog.setWindowTitle("選択");

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QListWidget *listWidget = new QListWidget;

	for (const auto &e : entries) {
		QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(e.label), listWidget);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
		item->setData(Qt::UserRole, QString::fromStdString(e.id));
	}
	layout->addWidget(listWidget);

	QPushButton *okButton = new QPushButton("OK");
	layout->addWidget(okButton);

	QObject::connect(okButton, &QPushButton::clicked, [&]() {
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
	}
	return result;
}


void gmain() {
	std::vector<Entry> list = {
		{"abc","aaaa1"},{"def","bbbb2"},{"ghi","cccc3"}
	};
	for (const std::string& e: selectItem(list)) {
		std::cout << e << "\n";
	}
}