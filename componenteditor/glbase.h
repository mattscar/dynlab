#ifndef GLBASE_H
#define GLBASE_H

// Declares the base of the graphical widget

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
