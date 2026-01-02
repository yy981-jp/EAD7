#pragma once
#include <windows.h>
#include <QtWidgets/QPushButton>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>
#include <QtWidgets/QFileDialog>

#include "../extern_inp_from.h"

class FileButton : public QPushButton {
	Q_OBJECT
public:
	using QPushButton::QPushButton;
	
	FileButton(QWidget *parent = nullptr) : QPushButton(parent) {
		setAcceptDrops(true);
		QObject::connect(this, &QPushButton::clicked, this, &FileButton::onPressed);
	}

protected:
	void dragEnterEvent(QDragEnterEvent *event) override {
		if (event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	void dropEvent(QDropEvent *event) override {
		if (event->mimeData()->hasUrls()) {
			mw::inp_from = INP_FROM::file;
			QString filePath = event->mimeData()->urls().first().toLocalFile();
			emit fileSelected(filePath);
		}
	}

signals:
	void fileSelected(const QString &filePath);

public slots:
	void onPressed() {
		QString filePath = QFileDialog::getOpenFileName(this, tr("ファイルを選択"));
		if (!filePath.isEmpty()) emit fileSelected(filePath);
	}
};
