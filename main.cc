#include <QApplication>
#include "mainwindow.h"

/*
This program updates the graphical interface
*/
int main(int argc, char *argv[]) {

  /* Configure application */
  QApplication app(argc, argv);
  app.setOrganizationName("Quiller Technologies LLC");
  app.setApplicationName("DynLab");

  /* Create window */
  MainWindow win;
  win.showMaximized();
  win.show();
  return app.exec();
}
