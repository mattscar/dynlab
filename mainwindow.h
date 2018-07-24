#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QActionGroup>
#include "componenteditor/tabeditor.h"
#include "navigator/navigator.h"
#include "propertybrowser/propertybrowser.h"

QT_BEGIN_NAMESPACE
class QSlider;
class GLWidget;
class QAction;
class QMenu;
class QDockWidget;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  MainWindow();

  // Draw actions
  QActionGroup *draw_group;
  QAction *selection_action;
  QAction *circle_action;
  QAction *rect_action;

  // Sim actions
  QAction *time_action;
  QAction *play_action;
  QAction *pause_action;
  QAction *stop_action;

  void maximizeEditor();

  PropertyBrowser *property_browser;

protected:
  void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent *event);

private slots:

// File menu slots
  void newFile();
  void open();
  bool save();
  bool saveAs();
  void print();

// Help menu slots
  void about();

private:

  void createFileActions();
  void createEditActions();
  void createViewActions();
  void createDrawActions();
  void createSimActions();
  void createHelpActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();

  // Editor maximized?
  bool editorMaximized;

  // Create windows
  QDockWidget *navigatorWidget;
  QDockWidget *console_widget;
  QDockWidget *browser_widget;
  TabEditor *tab_editor;
  Navigator *navigator;
  QTextEdit *console;

  // Current file
  QString curFile;
  QString image_dir;

  // Menus
  QMenu *file_menu;
  QMenu *edit_menu;
  QMenu *view_menu;
  QMenu *draw_menu;
  QMenu *sim_menu;
  QMenu *help_menu;

  // Toolbars
  QToolBar *file_bar;
  QToolBar *edit_bar;
  QToolBar *view_bar;
  QToolBar *sim_bar;
  QToolBar *draw_bar;
  QToolBar *help_bar;

  // File actions
  QAction *new_file_action;
  QAction *open_file_action;
  QAction *save_file_action;
  QAction *save_as_action;
  QAction *print_action;
  QAction *exit_action;

  // Edit actions
  QAction *cut_action;
  QAction *copy_action;
  QAction *paste_action;

  // View actions
  QAction *zoom_in_action;
  QAction *zoom_out_action;

  // Help
  QAction *about_action;

  QSlider *createSlider();
};

#endif
