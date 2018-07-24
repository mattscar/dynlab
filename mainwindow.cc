#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow() {

  // Access images used in user interface
  imageDir = QCoreApplication::applicationDirPath() + "/images/";

  // Create actions
  createFileActions();
  createEditActions();
  createViewActions();
  createDrawActions();
  createSimActions();
  createHelpActions();

  // Set central widget
  tabEditor = new TabEditor(this);
  setCentralWidget(tabEditor);

  // Configure navigator
  navigatorWidget = new QDockWidget(tr("Project Navigator"), this);
  navigatorWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  navigator = new Navigator(this);
  navigatorWidget->setWidget(navigator);
  navigatorWidget->setMaximumWidth(225);
  addDockWidget(Qt::LeftDockWidgetArea, navigatorWidget);

  // Configure console
  consoleWidget = new QDockWidget(tr("Console"), this);
  console = new QTextEdit(consoleWidget);
  consoleWidget->setWidget(console);
  consoleWidget->setMaximumHeight(75);
  addDockWidget(Qt::BottomDockWidgetArea, consoleWidget);

  // Configure property browser
  browserWidget = new QDockWidget(tr("Property Browser"), this);
  property_browser = new PropertyBrowser();
  browserWidget->setWidget(property_browser);
  browserWidget->setMinimumWidth(300);
  addDockWidget(Qt::RightDockWidgetArea, browserWidget);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // Connect navigator double-click event to tab editor
  connect(navigator, SIGNAL(doubleClicked(const QModelIndex &)), tabEditor, SLOT(createTab(const QModelIndex &)));
  // connect(tabEditor->tabBar(), SIGNAL(doubleClicked(const QModelIndex &)), tabEditor, SLOT(createTab(const QModelIndex &)));

  editorMaximized = false;

  createMenus();
  createToolBars();
  createStatusBar();

  setWindowTitle(tr("DynLab"));
  setWindowIcon(QIcon(imageDir + "logo.png"));
}

// Check whether document needs to be saved
static bool maybeSave() {
  return false;
}

// Create QActions for file operations
void MainWindow::createFileActions() {

  // Create new file
  newFileAction = new QAction(QIcon(imageDir + "new.png"), tr("&New"), this);
  newFileAction->setShortcuts(QKeySequence::New);
  newFileAction->setStatusTip(tr("Create a new project"));
  connect(newFileAction, SIGNAL(triggered()), this, SLOT(newFile()));

  // Open file
  openFileAction = new QAction(QIcon(imageDir + "open.png"), tr("&Open..."), this);
  openFileAction->setShortcuts(QKeySequence::Open);
  openFileAction->setStatusTip(tr("Open an existing project"));
  connect(openFileAction, SIGNAL(triggered()), this, SLOT(open()));

  // Save File
  saveFileAction = new QAction(QIcon(imageDir + "save.png"), tr("&Save"), this);
  saveFileAction->setShortcuts(QKeySequence::Save);
  saveFileAction->setStatusTip(tr("Save the project"));
  connect(saveFileAction, SIGNAL(triggered()), this, SLOT(save()));

  // Save as
  saveAsAction = new QAction(tr("Save &As..."), this);
  saveAsAction->setShortcuts(QKeySequence::SaveAs);
  saveAsAction->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  // Print
  printAction = new QAction(QIcon(imageDir + "print.png"), tr("&Print"), this);
  printAction->setShortcuts(QKeySequence::Print);
  printAction->setStatusTip(tr("Send the document to a printer"));
  connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

  // Exit
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcuts(QKeySequence::Quit);
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

// Create QActions for edit operations
void MainWindow::createEditActions() {

  // Cut
  cutAction = new QAction(QIcon(imageDir + "cut.png"), tr("Cut"), this);
  cutAction->setShortcuts(QKeySequence::Cut);
  cutAction->setStatusTip(tr("Cut"));
  //connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

  // Copy
  copyAction = new QAction(QIcon(imageDir + "copy.png"), tr("Copy"), this);
  copyAction->setShortcuts(QKeySequence::Copy);
  copyAction->setStatusTip(tr("Copy"));
  //connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

  // Paste
  pasteAction = new QAction(QIcon(imageDir + "paste.png"), tr("Paste"), this);
  pasteAction->setShortcuts(QKeySequence::Paste);
  pasteAction->setStatusTip(tr("Paste"));
  //connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
}

// Create QActions for drawing faces
void MainWindow::createDrawActions() {

  // Create selection action
  selectionAction = new QAction(QIcon(imageDir + "arrow.png"), tr("&Select..."), this);
  selectionAction->setStatusTip(tr("Select one or more objects"));
  selectionAction->setCheckable(true);

  // Create circle action
  circleAction = new QAction(QIcon(imageDir + "circle.png"), tr("&Circle..."), this);
  circleAction->setStatusTip(tr("Draw a circle"));
  circleAction->setCheckable(true);

  // Create rect action
  rectAction = new QAction(QIcon(imageDir + "rect.png"), tr("&Rectangle..."), this);
  rectAction->setStatusTip(tr("Draw a rectangle"));
  rectAction->setCheckable(true);

  // Create action group
  drawGroup = new QActionGroup(this);
  drawGroup->addAction(selectionAction);
  drawGroup->addAction(circleAction);
  drawGroup->addAction(rectAction);
  drawGroup->setEnabled(false);
}


// Create QActions for view operations
void MainWindow::createViewActions() {

  // Create zoom in action
  zoomInAction = new QAction(QIcon(imageDir + "zoomIn.png"), tr("Zoom in"), this);
  zoomInAction->setShortcuts(QKeySequence::ZoomIn);
  zoomInAction->setStatusTip(tr("Zoom in"));

  // Create zoom out action
  zoomOutAction = new QAction(QIcon(imageDir + "zoomOut.png"), tr("Zoom out"), this);
  zoomOutAction->setShortcuts(QKeySequence::ZoomOut);
  zoomOutAction->setStatusTip(tr("Zoom out"));
}

// Create actions related to timing and simulation
void MainWindow::createSimActions() {

  // Create time action
  timeAction = new QAction(QIcon(imageDir + "time.png"), tr("Time"), this);
  timeAction->setStatusTip(tr("Configure timing"));

  // Create play action
  playAction = new QAction(QIcon(imageDir + "play.png"), tr("Play"), this);
  playAction->setStatusTip(tr("Continue simulation"));

  // Create pause action
  pauseAction = new QAction(QIcon(imageDir + "pause.png"), tr("Pause"), this);
  pauseAction->setStatusTip(tr("Pause simulation"));

  // Create stop action
  stopAction = new QAction(QIcon(imageDir + "stop.png"), tr("Stop"), this);
  stopAction->setStatusTip(tr("Stop simulation"));
}

// Create QActions for help operations
void MainWindow::createHelpActions() {

  // Configure the About action in the help menu
  aboutAction = new QAction(QIcon(imageDir + "help.png"), tr("Help"), this);
  aboutAction->setStatusTip(tr("Provide assistance"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

// Assemble actions within main menu
void MainWindow::createMenus() {

  // Create file menu
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newFileAction);
  fileMenu->addAction(openFileAction);
  fileMenu->addAction(saveFileAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(printAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);
  menuBar()->addSeparator();

  // Create edit menu
  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(cutAction);
  editMenu->addAction(copyAction);
  editMenu->addAction(pasteAction);
  menuBar()->addSeparator();

  // Create view menu
  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAction);
  viewMenu->addAction(zoomOutAction);
  menuBar()->addSeparator();

  // Create draw menu
  drawMenu = menuBar()->addMenu(tr("&Draw"));
  drawMenu->addAction(selectionAction);
  drawMenu->addAction(circleAction);
  drawMenu->addAction(rectAction);
  menuBar()->addSeparator();

  // Create simulation menu
  simMenu = menuBar()->addMenu(tr("&Simulation"));
  simMenu->addAction(timeAction);
  simMenu->addAction(playAction);
  simMenu->addAction(pauseAction);
  simMenu->addAction(stopAction);
  menuBar()->addSeparator();

  // Create help menu
  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAction);
}

// Add entries to toolbars
void MainWindow::createToolBars() {

  // Create tool bar with file actions
  fileBar = addToolBar(tr("File"));
  fileBar->addAction(newFileAction);
  fileBar->addAction(openFileAction);
  fileBar->addAction(saveFileAction);
  fileBar->addAction(printAction);

  // Create tool bar with view actions
  viewBar = addToolBar(tr("View"));
  viewBar->addAction(zoomInAction);
  viewBar->addAction(zoomOutAction);

  // Create tool bar with draw actions
  drawBar = addToolBar(tr("Draw"));
  drawBar->addAction(selectionAction);
  drawBar->addAction(circleAction);
  drawBar->addAction(rectAction);

  // Create simulation tool bar
  simBar = addToolBar(tr("Simulate"));
  simBar->addAction(timeAction);
  simBar->addAction(playAction);
  simBar->addAction(pauseAction);
  //simBar->addAction(stopAction);
  menuBar()->addSeparator();

  // Create help tool bar
  helpBar = addToolBar(tr("Help"));
  helpBar->addAction(aboutAction);
}

void MainWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

// Respond to keypress
void MainWindow::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Escape)
    close();
  else
    QWidget::keyPressEvent(e);
}

// Close GUI
void MainWindow::closeEvent(QCloseEvent *event) {
  if (maybeSave()) {
    // writeSettings();
    event->accept();
  } else {
    event->accept();
  }
}

// Create new file
void MainWindow::newFile() {
  if (maybeSave()) {
    // textEdit->clear();
    // setCurrentFile("");
  }
}

// Open new file
void MainWindow::open() {
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
  /*  if (!fileName.isEmpty())
      loadFile(fileName); */
  }
}

// Save file
bool MainWindow::save() {
  if (curFile.isEmpty()) {
    return saveAs();
  }
  return false;
  /*else {
    return saveFile(curFile);
  }
  */
}

// Save file as
bool MainWindow::saveAs() {
  QString fileName = QFileDialog::getSaveFileName(this);
  if (fileName.isEmpty())
    return false;
  return false;
  // saveFile(fileName);
}

// Print document
void MainWindow::print() {
  QMessageBox::about(this, tr("Print"),
      tr("The print button tells the application to send the document to a printer."));
}


// Display help information
void MainWindow::about() {
  QMessageBox::about(this, tr("About Application"),
    tr("<b>DynLab</b> demonstrates how to "
    "simulate dynamic systems using GPU acceleration."));
}

// Maximize editor
void MainWindow::maximizeEditor() {
  if(!editorMaximized) {
    consoleWidget->setVisible(false);
    browserWidget->setVisible(false);
    navigatorWidget->setVisible(false);
    editorMaximized = true;
  } else {
    consoleWidget->setVisible(true);
    browserWidget->setVisible(true);
    navigatorWidget->setVisible(true);
    editorMaximized = false;
  }
}
