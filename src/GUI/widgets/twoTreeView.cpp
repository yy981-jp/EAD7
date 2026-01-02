#include "twoTreeView.h"

#include <string>
#include <vector>

#include <QtCore/QMimeData>

	TwoTreeView::TwoTreeView(QWidget *parent): QTreeView(parent), enable(false) {}
	
	void TwoTreeView::init(const bool isLeft_i, QStandardItemModel* model) {
		isLeft = isLeft_i;
		setModel(model);
		setDragEnabled(true);
		setAcceptDrops(true);
		setDropIndicatorShown(true);
		if (isLeft) {
			setDragDropMode(QAbstractItemView::InternalMove);
			// 親ノード（1層目）はドラッグ不可
			for (int i = 0; i < model->rowCount(); ++i) {
				QStandardItem *item = model->item(i);
				item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
			}
		} else {
			setDragDropMode(QAbstractItemView::DragDrop);
			setDefaultDropAction(Qt::MoveAction);
		}
		expandAll();
		enable = true;
	}

	std::vector<std::string> TwoTreeView::getFlatModel() const {
		std::vector<std::string> result;

		auto* m = qobject_cast<QStandardItemModel*>(model());
		if (!m) return result;

		for (int i = 0; i < m->rowCount(); ++i) {
			if (auto* item = m->item(i))
				result.push_back(item->text().toStdString());
		}
		return result;
	}
	
	QStandardItemModel* TwoTreeView::convModel(const std::string& label, const std::map<std::string, std::vector<std::string>>& i) {
		auto* qmodel = new QStandardItemModel;
		qmodel->setHorizontalHeaderLabels(QStringList() << QString::fromStdString(label));

		for (const auto& [root, items] : i) {
			auto* rootItem = new QStandardItem(QString::fromStdString(root));
			rootItem->setFlags(rootItem->flags() & ~Qt::ItemIsEditable);
			for (const auto& child : items) {
				auto* item = new QStandardItem(QString::fromStdString(child));
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
				rootItem->appendRow(item);
			}

			qmodel->appendRow(rootItem);
		}

		return qmodel;
	}

	QStandardItemModel* TwoTreeView::convModel(const std::string& label, const std::vector<std::string>& i) {
		auto* qmodel = new QStandardItemModel;
		qmodel->setHorizontalHeaderLabels(QStringList() << QString::fromStdString(label));

		for (const std::string& root : i) {
			QStandardItem* item = new QStandardItem(QString::fromStdString(root));
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			qmodel->appendRow(item);
		}

		return qmodel;
	}


	void TwoTreeView::dragEnterEvent(QDragEnterEvent *event) {
		event->acceptProposedAction();
	}

	void TwoTreeView::dragMoveEvent(QDragMoveEvent *event) {
		QModelIndex idx = indexAt(event->position().toPoint());
		if (isLeft) {
			// 左側: ドロップ不可（並び替えは許可）
			if (event->source() == this) {
				event->acceptProposedAction(); // 自分内の並び替えOK
			} else {
				event->ignore(); // 他からのドロップNG
			}
		} else {
			// 右側: 1層まで（子を持てない）
			if (idx.isValid()) {
				// すでにアイテムがある位置へのドロップ禁止
				event->ignore();
			} else {
				event->acceptProposedAction();
			}
		}
	}
#include <iostream>
	void TwoTreeView::dropEvent(QDropEvent *event) {
		if (isLeft) {
			std::cout << "D: dropEvent.left\n";
			// 左にドロップされたら右側から削除
			if (event->source() != this) {
				event->acceptProposedAction();

				// 右側モデルから削除処理
				auto src = qobject_cast<QTreeView*>(event->source());
				if (src) {
					QModelIndex idx = src->currentIndex();
					auto model = qobject_cast<QStandardItemModel*>(src->model());
					if (model) {
						QStandardItem *item = model->itemFromIndex(idx);
						if (item && item->parent()) {
							item->parent()->removeRow(idx.row());
						} else if (item) {
							model->removeRow(idx.row());
						}
					} else std::cout << "D: if model else\n";
				} else std::cout << "D: if src else\n";
			} else std::cout << "D: if event->source else\n";
		} else {
			std::cout << "D: dropEvent.right\n";
			// 通常ドロップ処理
			QTreeView::dropEvent(event);
		}
	}

	TwoTreeView::operator bool() const {
		return enable;
	}
