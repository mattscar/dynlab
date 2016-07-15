#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "../mainwindow.h"
#include "../spheredata.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "../fileinterface/colladainterface.h"

#include <QGLWidget>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QColor>
#include <QPoint>
#include <QDebug>
//#include <QUrl>
#include <QFileInfo>
#include <QTime>
#include <QTimer>

#include <fstream>
#include <iostream>
#include <iterator>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// OpenGL Math Library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenCL headers
#include <CL/cl_gl.h>
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"

enum ToolType {SELECTION, CIRCLE, RECTANGLE};

enum ToolState {NO_CLICK, FIRST_CLICK};

class GLWidget : public QGLWidget {
    Q_OBJECT

public:
  GLWidget(QWidget *parent = 0);
  ~GLWidget();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  static void addLine(float* vertices, GLint num_vertices);

public slots:
  void readProperties();
  void pauseSimulation();
  void playSimulation();
  void stopSimulation();

protected:

  // OpenGL event-handling functions
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  // Mouse response functions
  void mousePressEvent(QMouseEvent *event);
  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  /*
  void mouseMoveEvent(QMouseEvent *event);
  */

private:

  // Initialization functions
  void init_cl();
  GLuint init_shaders();
  std::string read_file(const char* filename);
  void compile_shader(GLint shader);
  void init_uniforms(GLuint program);
  void init_buffers(GLuint program);
  void init_physics();

  // Deallocation functions
  void deallocateCL();
  void deallocateGL();

  // Constants
  static const unsigned int kNumObjects = 28;
  static const unsigned int kObjectsPerRow = 7;

  // Sphere data
  struct SphereData* sphere_vec;

  // Sphere properties
  struct SphereProperties* sphere_props;

  // Number of indices per sphere
  GLsizei* count;

  // Indices per sphere
  GLvoid** indices;

  // Shader names
  static const char* kVertexShaderName;
  static const char* kFragmentShaderName;

  // Program names
  static const char* kMotionProgramFile;
  static const char* kPickSelectionProgramFile;

  // Kernel names
  static const char* kCollisionKernelName;
  static const char* kUpdateKernelName;
  static const char* kMotionKernelName;
  static const char* kPickSelectionKernelName;

  // OpenGL viewport size parameters
  const float kMinZ;
  const float kMaxZ;
  const float kMinRadius;
  const float kMaxRadius;

  // Physical simulation parameters
  const float kMinVelocity;
  const float kMaxVelocity;
  const float kMinAcceleration;
  const float kMaxAcceleration;

  // Color parameters
  const float kMinColor;
  const float kMaxColor;

  // OpenGL variables
  glm::mat4 modelview_matrix, mvp_matrix;   // The modelview matrices
  glm::mat4 mvp_inverse;                    // Inverse of the MVP matrix
  std::vector<ColGeom> geom_vec;            // Vector containing COLLADA meshes
  GLuint vao, ibo, ubo, vbos[2];            // OpenGL buffer objects
  GLint mvp_location, color_location;       // Index of the MVP/color uniforms
  float half_height, half_width;            // Window dimensions divided in half
  size_t num_vertices, num_triangles;       // Number of vertices and triangles in the rendering

  // Pick-selection information
  float *pick_result;                       // Pick selection result
  size_t num_groups;                        // Size of pick selection result
  const glm::vec3 selected_color;			// The color when selected
  unsigned int selected_object;             // The selected object

  // OpenGL data
  glm::vec3 *vertex_data;
  float *normal_data;
  unsigned short* index_data;

  // Timing and physics
  QTime* timer;
  int previous_time, collide, state;

  // OpenCL variables
  cl_platform_id platform;
  cl_device_id device;
  cl_context dev_context;
  cl_program motion_program, pick_selection_program;
  cl_command_queue queue;
  cl_kernel collision_kernel, update_kernel, motion_kernel, pick_selection_kernel;
  cl_mem vbo_memobj, ibo_memobj, sphere_memobj, pick_buffer;
  size_t obj_local_size, obj_global_size, vertex_local_size, vertex_global_size, pick_local_size, pick_global_size;

  // The main window
  MainWindow *win;

  // The current tool
  ToolType current_tool;
  ToolState current_state;

  // Mouse position
  int click_x, click_y;

  // Program data
  std::ifstream programFile;
  std::string programString;

  int xRot;
  int yRot;
  int zRot;
  QPoint lastPos;
  QColor qtGreen;
  QColor qtPurple;

private slots:

  // Idle function - execute kernels
  void update_vertices();

  // Make actions current
  void makeSelectionActionActive();
  void makeCircleActionActive();
  void makeRectActionActive();
};

#endif
