#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {

  /* Configure application */
  QApplication app(argc, argv);
  app.setOrganizationName("Eclipse Engineering LLC");
  app.setApplicationName("DynLab");

  /* Create window */
  MainWindow win;
  win.showMaximized();
  win.show();
  return app.exec();
}
