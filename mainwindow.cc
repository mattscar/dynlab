#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow() {

  // Access images used in user interface
  image_dir = QCoreApplication::applicationDirPath() + "/images/";

  // Create actions
  createFileActions();
  createEditActions();
  createViewActions();
  createDrawActions();
  createSimActions();
  createHelpActions();

  // Set central widget
  tab_editor = new TabEditor(this);
  setCentralWidget(tab_editor);

  // Configure navigator
  navigatorWidget = new QDockWidget(tr("Project Navigator"), this);
  navigatorWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  navigator = new Navigator(this);
  navigatorWidget->setWidget(navigator);
  navigatorWidget->setMaximumWidth(225);
  addDockWidget(Qt::LeftDockWidgetArea, navigatorWidget);

  // Configure console
  console_widget = new QDockWidget(tr("Console"), this);
  console = new QTextEdit(console_widget);
  console_widget->setWidget(console);
  console_widget->setMaximumHeight(75);
  addDockWidget(Qt::BottomDockWidgetArea, console_widget);

  // Configure property browser
  browser_widget = new QDockWidget(tr("Property Browser"), this);
  property_browser = new PropertyBrowser();
  browser_widget->setWidget(property_browser);
  browser_widget->setMinimumWidth(300);
  addDockWidget(Qt::RightDockWidgetArea, browser_widget);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // Connect navigator double-click event to tab editor
  connect(navigator, SIGNAL(doubleClicked(const QModelIndex &)), tab_editor, SLOT(createTab(const QModelIndex &)));
  // connect(tabEditor->tabBar(), SIGNAL(doubleClicked(const QModelIndex &)), tabEditor, SLOT(createTab(const QModelIndex &)));

  editorMaximized = false;

  createMenus();
  createToolBars();
  createStatusBar();

  setWindowTitle(tr("DynLab"));
  setWindowIcon(QIcon(image_dir + "logo.png"));
}

// Check whether document needs to be saved
static bool maybeSave() {
  return false;
}

// Create QActions for file operations
void MainWindow::createFileActions() {

  // Create new file
  new_file_action = new QAction(QIcon(image_dir + "new.png"), tr("&New"), this);
  new_file_action->setShortcuts(QKeySequence::New);
  new_file_action->setStatusTip(tr("Create a new project"));
  connect(new_file_action, SIGNAL(triggered()), this, SLOT(newFile()));

  // Open file
  open_file_action = new QAction(QIcon(image_dir + "open.png"), tr("&Open..."), this);
  open_file_action->setShortcuts(QKeySequence::Open);
  open_file_action->setStatusTip(tr("Open an existing project"));
  connect(open_file_action, SIGNAL(triggered()), this, SLOT(open()));

  // Save File
  save_file_action = new QAction(QIcon(image_dir + "save.png"), tr("&Save"), this);
  save_file_action->setShortcuts(QKeySequence::Save);
  save_file_action->setStatusTip(tr("Save the project"));
  connect(save_file_action, SIGNAL(triggered()), this, SLOT(save()));

  // Save as
  save_as_action = new QAction(tr("Save &As..."), this);
  save_as_action->setShortcuts(QKeySequence::SaveAs);
  save_as_action->setStatusTip(tr("Save the document under a new name"));
  connect(save_as_action, SIGNAL(triggered()), this, SLOT(saveAs()));

  // Print
  print_action = new QAction(QIcon(image_dir + "print.png"), tr("&Print"), this);
  print_action->setShortcuts(QKeySequence::Print);
  print_action->setStatusTip(tr("Send the document to a printer"));
  connect(print_action, SIGNAL(triggered()), this, SLOT(print()));

  // Exit
  exit_action = new QAction(tr("E&xit"), this);
  exit_action->setShortcuts(QKeySequence::Quit);
  exit_action->setStatusTip(tr("Exit the application"));
  connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));
}

// Create QActions for edit operations
void MainWindow::createEditActions() {

  // Cut
  cut_action = new QAction(QIcon(image_dir + "cut.png"), tr("Cut"), this);
  cut_action->setShortcuts(QKeySequence::Cut);
  cut_action->setStatusTip(tr("Cut"));
  //connect(cut_action, SIGNAL(triggered()), this, SLOT(cut()));

  // Copy
  copy_action = new QAction(QIcon(image_dir + "copy.png"), tr("Copy"), this);
  copy_action->setShortcuts(QKeySequence::Copy);
  copy_action->setStatusTip(tr("Copy"));
  //connect(copy_action, SIGNAL(triggered()), this, SLOT(copy()));

  // Paste
  paste_action = new QAction(QIcon(image_dir + "paste.png"), tr("Paste"), this);
  paste_action->setShortcuts(QKeySequence::Paste);
  paste_action->setStatusTip(tr("Paste"));
  //connect(paste_action, SIGNAL(triggered()), this, SLOT(paste()));
}

// Create QActions for drawing faces
void MainWindow::createDrawActions() {

  // Create selection action
  selection_action = new QAction(QIcon(image_dir + "arrow.png"), tr("&Select..."), this);
  selection_action->setStatusTip(tr("Select one or more objects"));
  selection_action->setCheckable(true);

  // Create circle action
  circle_action = new QAction(QIcon(image_dir + "circle.png"), tr("&Circle..."), this);
  circle_action->setStatusTip(tr("Draw a circle"));
  circle_action->setCheckable(true);

  // Create rect action
  rect_action = new QAction(QIcon(image_dir + "rect.png"), tr("&Rectangle..."), this);
  rect_action->setStatusTip(tr("Draw a rectangle"));
  rect_action->setCheckable(true);

  // Create action group
  draw_group = new QActionGroup(this);
  draw_group->addAction(selection_action);
  draw_group->addAction(circle_action);
  draw_group->addAction(rect_action);
  draw_group->setEnabled(false);
}


// Create QActions for view operations
void MainWindow::createViewActions() {

  // Create zoom in action
  zoom_in_action = new QAction(QIcon(image_dir + "zoom_in.png"), tr("Zoom in"), this);
  zoom_in_action->setShortcuts(QKeySequence::ZoomIn);
  zoom_in_action->setStatusTip(tr("Zoom in"));

  // Create zoom out action
  zoom_out_action = new QAction(QIcon(image_dir + "zoom_out.png"), tr("Zoom out"), this);
  zoom_out_action->setShortcuts(QKeySequence::ZoomOut);
  zoom_out_action->setStatusTip(tr("Zoom out"));
}

// Create actions related to timing and simulation
void MainWindow::createSimActions() {

  // Create time action
  time_action = new QAction(QIcon(image_dir + "time.png"), tr("Time"), this);
  time_action->setStatusTip(tr("Configure timing"));

  // Create play action
  play_action = new QAction(QIcon(image_dir + "play.png"), tr("Play"), this);
  play_action->setStatusTip(tr("Continue simulation"));

  // Create pause action
  pause_action = new QAction(QIcon(image_dir + "pause.png"), tr("Pause"), this);
  pause_action->setStatusTip(tr("Pause simulation"));

  // Create stop action
  stop_action = new QAction(QIcon(image_dir + "stop.png"), tr("Stop"), this);
  stop_action->setStatusTip(tr("Stop simulation"));
}

// Create QActions for help operations
void MainWindow::createHelpActions() {

  // Configure the About action in the help menu
  about_action = new QAction(QIcon(image_dir + "help.png"), tr("Help"), this);
  about_action->setStatusTip(tr("Provide assistance"));
  connect(about_action, SIGNAL(triggered()), this, SLOT(about()));
}

// Assemble actions within main menu
void MainWindow::createMenus() {

  // Create file menu
  file_menu = menuBar()->addMenu(tr("&File"));
  file_menu->addAction(new_file_action);
  file_menu->addAction(open_file_action);
  file_menu->addAction(save_file_action);
  file_menu->addAction(save_as_action);
  file_menu->addSeparator();
  file_menu->addAction(print_action);
  file_menu->addSeparator();
  file_menu->addAction(exit_action);
  menuBar()->addSeparator();

  // Create edit menu
  edit_menu = menuBar()->addMenu(tr("&Edit"));
  edit_menu->addAction(cut_action);
  edit_menu->addAction(copy_action);
  edit_menu->addAction(paste_action);
  menuBar()->addSeparator();

  // Create view menu
  view_menu = menuBar()->addMenu(tr("&View"));
  view_menu->addAction(zoom_in_action);
  view_menu->addAction(zoom_out_action);
  menuBar()->addSeparator();

  // Create draw menu
  draw_menu = menuBar()->addMenu(tr("&Draw"));
  draw_menu->addAction(selection_action);
  draw_menu->addAction(circle_action);
  draw_menu->addAction(rect_action);
  menuBar()->addSeparator();

  // Create simulation menu
  sim_menu = menuBar()->addMenu(tr("&Simulation"));
  sim_menu->addAction(time_action);
  sim_menu->addAction(play_action);
  sim_menu->addAction(pause_action);
  sim_menu->addAction(stop_action);
  menuBar()->addSeparator();

  // Create help menu
  help_menu = menuBar()->addMenu(tr("&Help"));
  help_menu->addAction(about_action);
}

// Add entries to toolbars
void MainWindow::createToolBars() {

  // Create tool bar with file actions
  file_bar = addToolBar(tr("File"));
  file_bar->addAction(new_file_action);
  file_bar->addAction(open_file_action);
  file_bar->addAction(save_file_action);
  file_bar->addAction(print_action);

  // Create tool bar with view actions
  view_bar = addToolBar(tr("View"));
  view_bar->addAction(zoom_in_action);
  view_bar->addAction(zoom_out_action);

  // Create tool bar with draw actions
  draw_bar = addToolBar(tr("Draw"));
  draw_bar->addAction(selection_action);
  draw_bar->addAction(circle_action);
  draw_bar->addAction(rect_action);

  // Create simulation tool bar
  sim_bar = addToolBar(tr("Simulate"));
  sim_bar->addAction(time_action);
  sim_bar->addAction(play_action);
  sim_bar->addAction(pause_action);
  //sim_bar->addAction(stop_action);
  menuBar()->addSeparator();

  // Create help tool bar
  help_bar = addToolBar(tr("Help"));
  help_bar->addAction(about_action);
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
    console_widget->setVisible(false);
    browser_widget->setVisible(false);
    navigatorWidget->setVisible(false);
    editorMaximized = true;
  } else {
    console_widget->setVisible(true);
    browser_widget->setVisible(true);
    navigatorWidget->setVisible(true);
    editorMaximized = false;
  }
}
