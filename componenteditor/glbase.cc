#include "glbase.h"

GLBase::GLBase(QWidget* parent) : QScrollArea(parent) {

  glwidget = new GLWidget(this);
  setWidget(glwidget);
  setWidgetResizable(true);
};

GLWidget* GLBase::getGLWidget() {
  return glwidget;
}
