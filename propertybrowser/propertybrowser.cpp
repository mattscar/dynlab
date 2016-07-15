 #include "propertybrowser.h"
#include "qteditorfactory.h"

PropertyBrowser::PropertyBrowser() : QtTreePropertyBrowser() {

  QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory();
  QtLineEditFactory *lineEditFactory = new QtLineEditFactory();
  QtDoubleSpinBoxFactory *doubleSpinBoxFactory = new QtDoubleSpinBoxFactory();

  // Display ID of selected object
  idManager = new QtIntPropertyManager;
  selected_id = idManager->addProperty("Object ID:");
  selected_id->setToolTip("ID of selected object");
  idManager->setValue(selected_id, -1);
  //idManager->setRange(priority, 1, 5);
  setFactoryForManager(idManager, spinBoxFactory);
  addProperty(selected_id);

  // Display file of selected object;
  filenameManager = new QtStringPropertyManager();
  filename = filenameManager->addProperty("File name:");
  filename->setToolTip("File name of selected object");
  filenameManager->setValue(filename, QString(""));
  setFactoryForManager(filenameManager, lineEditFactory);
  addProperty(filename);

  // Display radius of selected object;
  radiusManager = new QtDoublePropertyManager();
  radius = radiusManager->addProperty("Radius:");
  radius->setToolTip("Radius of selected object");
  radiusManager->setValue(radius, 0.0);
  setFactoryForManager(radiusManager, doubleSpinBoxFactory);
  addProperty(radius);

  // Display radius of selected object;
  massManager = new QtDoublePropertyManager();
  mass = massManager->addProperty("Mass:");
  mass->setToolTip("Mass of selected object");
  massManager->setValue(mass, 0.0);
  setFactoryForManager(massManager, doubleSpinBoxFactory);
  addProperty(mass);

  // Display position of selected object;
  positionManager = new QtStringPropertyManager();
  position = positionManager->addProperty("Position:");
  position->setToolTip("Position of selected object");
  positionManager->setValue(position, QString(""));
  setFactoryForManager(positionManager, lineEditFactory);
  addProperty(position);

  // Display velocity of selected object;
  velocityManager = new QtStringPropertyManager();
  velocity = velocityManager->addProperty("Velocity:");
  velocity->setToolTip("Velocity of selected object");
  velocityManager->setValue(velocity, QString(""));
  setFactoryForManager(velocityManager, lineEditFactory);
  addProperty(velocity);

  // Display acceleration of selected object;
  accelerationManager = new QtStringPropertyManager();
  acceleration = accelerationManager->addProperty("Acceleration:");
  acceleration->setToolTip("Acceleration of selected object");
  accelerationManager->setValue(acceleration, QString(""));
  setFactoryForManager(accelerationManager, lineEditFactory);
  addProperty(acceleration);
}

void PropertyBrowser::setSphereData(struct SphereData *data, struct SphereProperties *props) {

  idManager->setValue(selected_id, props->id);
  filenameManager->setValue(filename, props->filename);
  radiusManager->setValue(radius, data->radius);
  massManager->setValue(mass, props->mass);
  positionManager->setValue(position, QString("(%1, %2, %3)").arg(data->center.x, 0, 'g', 4).arg(data->center.y, 0, 'g', 4).arg(data->center.z, 0, 'g', 4));
  velocityManager->setValue(velocity, QString("(%1, %2, %3)").arg(data->new_velocity.x, 0, 'g', 4).arg(data->new_velocity.y, 0, 'g', 4).arg(data->new_velocity.z, 0, 'g', 4));
  accelerationManager->setValue(acceleration, QString("(%1, %2, %3)").arg(data->acceleration.x, 0, 'g', 4).arg(data->acceleration.y, 0, 'g', 4).arg(data->acceleration.z, 0, 'g', 4));
}
