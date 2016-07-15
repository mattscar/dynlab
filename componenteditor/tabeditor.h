#ifndef TABEDITOR_H
#define TABEDITOR_H

// #include "../fileinterface/colladainterface.h"

#include <QTabWidget>
#include <QMainWindow>
#include <QModelIndex>
#include <QDebug>

class TabEditor : public QTabWidget {

  Q_OBJECT

public:
  TabEditor(QMainWindow* parent = 0);

public slots:
  void createTab(const QModelIndex& index);

protected:
  bool eventFilter(QObject*, QEvent*);

private:
  QTabBar* tabbar;
};

#endif
