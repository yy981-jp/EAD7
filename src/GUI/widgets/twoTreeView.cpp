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
			// 隕ｪ繝弱・繝会ｼ・螻､逶ｮ・峨・繝峨Λ繝・げ荳榊庄
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
			// 蟾ｦ蛛ｴ: 繝峨Ο繝・・荳榊庄・井ｸｦ縺ｳ譖ｿ縺医・險ｱ蜿ｯ・・
			if (event->source() == this) {
				event->acceptProposedAction(); // 閾ｪ蛻・・縺ｮ荳ｦ縺ｳ譖ｿ縺・K
			} else {
				event->ignore(); // 莉悶°繧峨・繝峨Ο繝・・NG
			}
		} else {
			// 蜿ｳ蛛ｴ: 1螻､縺ｾ縺ｧ・亥ｭ舌ｒ謖√※縺ｪ縺・ｼ・
			if (idx.isValid()) {
				// 縺吶〒縺ｫ繧｢繧､繝・Β縺後≠繧倶ｽ咲ｽｮ縺ｸ縺ｮ繝峨Ο繝・・遖∵ｭ｢
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
			// 蟾ｦ縺ｫ繝峨Ο繝・・縺輔ｌ縺溘ｉ蜿ｳ蛛ｴ縺九ｉ蜑企勁
			if (event->source() != this) {
				event->acceptProposedAction();

				// 蜿ｳ蛛ｴ繝｢繝・Ν縺九ｉ蜑企勁蜃ｦ逅・
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
			// 騾壼ｸｸ繝峨Ο繝・・蜃ｦ逅・
			QTreeView::dropEvent(event);
		}
	}

	TwoTreeView::operator bool() const {
		return enable;
	}
