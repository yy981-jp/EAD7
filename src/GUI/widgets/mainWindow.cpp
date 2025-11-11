#include "mainwindow.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include "../gui.h"
MainWindow::MainWindow() {
	ui = new Ui::MainWindow;
	ui->setupUi(this);
	if (!ui) crashReport("MainWindow::MainWindow()::ui==nullptr");

	// 右パネルを作成
	aui = new Ui::AdminWindow;
	aui->setupUi(ui->adminWidget);
	ui->adminWidget->hide(); // 初期は非表示

	// トグルボタンに接続（UI内のボタンを想定）
	connect(ui->adminMode, &QCheckBox::checkStateChanged, this, &MainWindow::toggleRightPanel);
}

void MainWindow::toggleRightPanel(const bool adminMode) {
	if (!ui->adminWidget) return;
	// if (adminMode) ui->adminWidget->resize(800,600);
	// if (adminMode) ui->adminWidgetDock->resize(800,600);
	ui->adminWidget->setVisible(adminMode);
}
