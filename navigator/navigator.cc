#include "navigator.h"

Navigator::Navigator(QMainWindow *parent) : QTreeView(parent) {

  model = new QFileSystemModel;
  model->setSupportedDragActions(Qt::CopyAction);
  QString workspace_dir = QCoreApplication::applicationDirPath() + "/workspace";
  model->setRootPath(workspace_dir);
  setModel(model);
  QModelIndex index = model->index(workspace_dir);
  setRootIndex(index);
  setDragEnabled(true);
  setDragDropMode(QAbstractItemView::DragDrop);
  setColumnHidden(1, true);
  setColumnHidden(2, true);
  setColumnHidden(3, true);
}

void Navigator::dragMoveEvent(QDragMoveEvent* event) {
  event->accept();
}

