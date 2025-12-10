#pragma once

#include <string>
#include <vector>

#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include <QtGui/QStandardItemModel>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>

class TwoTreeView : public QTreeView {
	Q_OBJECT
public:
	TwoTreeView(QWidget *parent = nullptr);
	void init(const bool isLeft, QStandardItemModel* model);
	bool isLeft;
	std::vector<std::string> getFlatModel() const;
	static QStandardItemModel* convModel(const std::string& label, const std::map<std::string, std::vector<std::string>>& i);
	static QStandardItemModel* convModel(const std::string& label, const std::vector<std::string>& i);

protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
};
