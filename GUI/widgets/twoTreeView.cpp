#include "twoTreeView.h"

#include <string>
#include <vector>

#include <QtCore/QMimeData>

	TwoTreeView::TwoTreeView(const bool isLeft, QStandardItemModel* model, QWidget *parent)
		: QTreeView(parent), isLeft(isLeft) {
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
			for (const auto& child : items) {
				rootItem->appendRow(new QStandardItem(QString::fromStdString(child)));
			}

			qmodel->appendRow(rootItem);
		}

		return qmodel;
	}

	QStandardItemModel* TwoTreeView::convModel(const std::string& label, const std::vector<std::string>& i) {
		auto* qmodel = new QStandardItemModel;
		qmodel->setHorizontalHeaderLabels(QStringList() << QString::fromStdString(label));

		for (const std::string& root : i) {
			qmodel->appendRow(new QStandardItem(QString::fromStdString(root)));
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

	void TwoTreeView::dropEvent(QDropEvent *event) {
		if (isLeft) {
			// 左にドロップされたら右側から削除
			if (event->source() != this) {
				event->acceptProposedAction();

				// 右側モデルから削除処理
				auto src = qobject_cast<QTreeView *>(event->source());
				if (src) {
					QModelIndex idx = src->currentIndex();
					auto model = qobject_cast<QStandardItemModel *>(src->model());
					if (model) {
						QStandardItem *item = model->itemFromIndex(idx);
						if (item && item->parent()) {
							item->parent()->removeRow(idx.row());
						} else if (item) {
							model->removeRow(idx.row());
						}
					}
				}
			}
		} else {
			// 通常ドロップ処理
			QTreeView::dropEvent(event);
		}
	}
