#pragma once
#include <windows.h>
#include <QtWidgets/QPushButton>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>
#include <QtWidgets/QFileDialog>

class FileButton : public QPushButton {
	Q_OBJECT
public:
	using QPushButton::QPushButton;

protected:
	void dragEnterEvent(QDragEnterEvent *event) override {
		if (event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	void dropEvent(QDropEvent *event) override {
		if (event->mimeData()->hasUrls()) {
			QString filePath = event->mimeData()->urls().first().toLocalFile();
			emit fileSelected(filePath);
		}
	}

	void mousePressEvent(QMouseEvent *event) override {
		if (event->button() == Qt::LeftButton) {
			QString filePath = QFileDialog::getOpenFileName(this, "ファイルを選択");
			if (!filePath.isEmpty()) {
				emit fileSelected(filePath);
			}
		}
		// 通常のQPushButtonの処理も呼ぶ
		// QPushButton::mousePressEvent(event);
	}

signals:
	void fileSelected(const QString &filePath);
};
