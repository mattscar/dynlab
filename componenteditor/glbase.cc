#include "glbase.h"

// Creates the underlying graphical object for the editor

GLBase::GLBase(QWidget* parent) : QScrollArea(parent) {

  glwidget = new GLWidget(this);
  setWidget(glwidget);
  setWidgetResizable(true);
};

GLWidget* GLBase::getGLWidget() {
  return glwidget;
}
