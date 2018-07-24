#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"
#include "../spheredata.h"

#include <QDebug>

class PropertyBrowser : public QtTreePropertyBrowser {

  Q_OBJECT

public:
  PropertyBrowser();

  void setSphereData(struct SphereData*, struct SphereProperties*);

private:
  // Display ID of selected object
  QtIntPropertyManager *idManager;
  QtProperty *selected_id;

  // Display filename of selected object
  QtStringPropertyManager *filenameManager;
  QtProperty *filename;

  // Display radius of selected object
  QtDoublePropertyManager *radiusManager;
  QtProperty *radius;

  // Display radius of selected object
  QtDoublePropertyManager *massManager;
  QtProperty *mass;

  // Display position of selected object
  QtStringPropertyManager *positionManager;
  QtProperty *position;

  // Display velocity of selected object
  QtStringPropertyManager *velocityManager;
  QtProperty *velocity;

  // Display acceleration of selected object
  QtStringPropertyManager *accelerationManager;
  QtProperty *acceleration;
};

#endif
