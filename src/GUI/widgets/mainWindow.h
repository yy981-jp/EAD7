#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

#include "../def.h"


class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow();

private slots:
	void toggleRightPanel(const bool adminMode);
};
