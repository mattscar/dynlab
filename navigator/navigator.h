#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QTreeView>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QCoreApplication>
#include <QDragMoveEvent>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE
class QFileSystemModel;
QT_END_NAMESPACE

class Navigator : public QTreeView {

  Q_OBJECT

public:
  Navigator(QMainWindow* parent = 0);

protected:
  void dragMoveEvent(QDragMoveEvent *event);

private:
  QFileSystemModel *model;
};

#endif
