#include "tabeditor.h"
#include "glbase.h"
#include "../mainwindow.h"
#include <QScrollArea>

TabEditor::TabEditor(QMainWindow *parent) : QTabWidget(parent) {

  setTabsClosable(true);
  QScrollArea* area = new QScrollArea(this);
  area->setWidget(new GLWidget(this));
  area->setWidgetResizable(true);
  //addTab(new GLBase(NULL, this), tr("OpenGL"));
  addTab(area, tr("Spheres in Motion"));
  tabbar = this->tabBar();
  tabbar->installEventFilter(this);
}

void TabEditor::createTab(const QModelIndex& index) {

  addTab(new GLBase(this), index.data(Qt::DisplayRole).toString());
  // ColladaInterface::readGraphics((QWidget*)this, QString("Hey now!")),
  setCurrentIndex(count()-1);
}

bool TabEditor::eventFilter(QObject* obj, QEvent* e) {

  bool result = QObject::eventFilter(obj, e);

  if (obj == tabbar && e->type() == QEvent::MouseButtonDblClick) {

    ((MainWindow*)parent())->maximizeEditor();
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    int index = tabbar->tabAt(me->pos());
    if (index == -1)
      return result;
    this->showFullScreen();
    return true;
  }

  return result;
}
