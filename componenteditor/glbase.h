#ifndef GLBASE_H
#define GLBASE_H

#include "glwidget.h"

#include <QScrollArea>

class GLBase : public QScrollArea {

  Q_OBJECT

public:
  GLBase(QWidget *parent = 0);
  GLWidget* getGLWidget();

private:
  GLWidget* glwidget;
};

#endif
