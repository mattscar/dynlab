#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#include "glwidget.h"

// Names of shader files
const char* GLWidget::kVertexShaderName = "shaders/dynlab.vert";
const char* GLWidget::kFragmentShaderName = "shaders/dynlab.frag";

// Names of program files
const char* GLWidget::kMotionProgramFile = "kernels/motion.cl";
const char* GLWidget::kPickSelectionProgramFile = "kernels/pick_selection.cl";

// Names of kernel functions
const char* GLWidget::kCollisionKernelName = "collision_detection";
const char* GLWidget::kUpdateKernelName = "update";
const char* GLWidget::kMotionKernelName = "motion";
const char* GLWidget::kPickSelectionKernelName = "pick_selection";

GLWidget::GLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), kMinZ(2.5f), kMaxZ(20.0f),
  kMinRadius(0.3f), kMaxRadius(0.8f), kMinVelocity(-0.5f), kMaxVelocity(0.5f), kMinAcceleration(-0.4f),
  kMaxAcceleration(0.4f), kMinColor(0.2f), kMaxColor(0.8f), selected_color(glm::vec3(1.0f, 1.0f, 1.0f)),
  selected_object(UINT_MAX), collide(0), state(0) {

  makeCurrent();
  setAcceptDrops(true);

  // Configure tool settings
  win = static_cast<MainWindow*>(parent->parent());
  win->draw_group->setEnabled(true);

  // Connect draw actions
  connect(win->selection_action, SIGNAL(triggered()), this, SLOT(makeSelectionActionActive()));
  connect(win->circle_action, SIGNAL(triggered()), this, SLOT(makeCircleActionActive()));
  connect(win->rect_action, SIGNAL(triggered()), this, SLOT(makeRectActionActive()));
  win->selection_action->setChecked(true);
  current_tool = SELECTION;

  // Connect simulation actions
  connect(win->pause_action, SIGNAL(triggered()), this, SLOT(pauseSimulation()));
  connect(win->play_action, SIGNAL(triggered()), this, SLOT(playSimulation()));
  connect(win->stop_action, SIGNAL(triggered()), this, SLOT(stopSimulation()));

  // Configure tool state
  current_state = NO_CLICK;

  // Read graphic data
  ColladaInterface::readGeometries(&geom_vec, "sphere.dae");
  num_vertices = geom_vec[0].map["POSITION"].size/12;
  num_triangles = geom_vec[0].index_count/3;
}

GLWidget::~GLWidget() {

  // Deallocate mesh data
  ColladaInterface::freeGeometries(&geom_vec);

  deallocateGL();
  deallocateCL();
}

void GLWidget::deallocateGL() {

  glFinish();

  // Deallocate arrays
  delete(vertex_data);
  delete(normal_data);
  delete(index_data);

  if(pick_result != NULL)
    delete(pick_result);

  // Deallocate OpenGL objects
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(2, vbos);
  glDeleteBuffers(1, &vao);
  glDeleteBuffers(1, &ubo);

  clReleaseKernel(collision_kernel);
  clReleaseKernel(update_kernel);
  clReleaseKernel(motion_kernel);
}

void GLWidget::deallocateCL() {

  // Deallocate OpenCL resources
  clReleaseKernel(collision_kernel);
  clReleaseKernel(update_kernel);
  clReleaseKernel(motion_kernel);
  clReleaseKernel(pick_selection_kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(motion_program);
  clReleaseProgram(pick_selection_program);
  clReleaseContext(dev_context);
  clReleaseMemObject(sphere_memobj);
  clReleaseMemObject(pick_buffer);
}

// Initialize OpenGL data structures
void GLWidget::initializeGL() {

  // Sphere data
  sphere_vec = new SphereData[kNumObjects];

  // Sphere properties
  sphere_props = new SphereProperties[kNumObjects];

  // Set background color
  glClearColor(0.0f, 0.820f, 0.8f, 1.0f);

  // Configure culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);

  // Start GLEW processing
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    qDebug() << "Can't initialize glew.\n";
  }

  // Initialize physical parameters
  init_physics();

  // Access and compile shaders
  GLuint program = init_shaders();

  // Create and initialize buffers
  init_buffers(program);

  // Create and initialize uniform data elements
  init_uniforms(program);

  // Create and initialize OpenCL structures
  init_cl();

  // Start main timer
  timer = new QTime();
  timer->start();
  previous_time = 0;

  // Start idle timer
  QTimer *idle_timer = new QTimer(this);
  connect(idle_timer, SIGNAL(timeout()), this, SLOT(update_vertices()));
  idle_timer->start();

  // Start property polling
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(readProperties()));
  timer->start(150);
}

// Initialize physical parameters
void GLWidget::init_physics() {

  srand(time(NULL));
  for(unsigned i=0; i<kNumObjects; i++) {
    sphere_vec[i].radius = static_cast<float>(rand())/RAND_MAX * (kMaxRadius - kMinRadius) + kMinRadius;
    sphere_vec[i].center = glm::vec3(kMaxRadius * 3.0f * ((i % kObjectsPerRow) + 1),
                                     kMaxRadius * 3.0f * ((i / kObjectsPerRow) + 1),
                                     -3.0f);
    sphere_vec[i].old_velocity = glm::vec4(1.0f*rand()/RAND_MAX * (kMaxVelocity - kMinVelocity) + kMinVelocity,
                                           1.0f*rand()/RAND_MAX * (kMaxVelocity - kMinVelocity) + kMinVelocity,
                                           1.0f*rand()/RAND_MAX * (kMaxVelocity - kMinVelocity) + kMinVelocity,
                                           0.0f);
    sphere_vec[i].new_velocity = sphere_vec[i].old_velocity;
    sphere_vec[i].acceleration = glm::vec4(1.0f*rand()/RAND_MAX * (kMaxAcceleration - kMinAcceleration) + kMinAcceleration,
                                           1.0f*rand()/RAND_MAX * (kMaxAcceleration - kMinAcceleration) + kMinAcceleration,
                                           1.0f*rand()/RAND_MAX * (kMaxAcceleration - kMinAcceleration) + kMinAcceleration,
                                           0.0f);
    sphere_vec[i].displacement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Set sphere properties
    sphere_props[i].id = static_cast<int>(i);
    sphere_props[i].color = glm::vec3(static_cast<float>(rand())/RAND_MAX * (kMaxColor - kMinColor) + kMinColor,
    		                          static_cast<float>(rand())/RAND_MAX * (kMaxColor - kMinColor) + kMinColor,
    		                          static_cast<float>(rand())/RAND_MAX * (kMaxColor - kMinColor) + kMinColor);
    sphere_props[i].filename = QString("sphere.dae");
    sphere_props[i].mass = 3.1f * sphere_vec[i].radius;
  }
}

// Initialize shader data
GLuint GLWidget::init_shaders() {

  GLuint vs, fs, prog;
  std::string vs_source, fs_source;
  const char *vs_chars, *fs_chars;
  GLint vs_length, fs_length;

  // Create shader descriptors
  vs = glCreateShader(GL_VERTEX_SHADER);
  fs = glCreateShader(GL_FRAGMENT_SHADER);

  // Read shader text from files
  vs_source = read_file(kVertexShaderName);
  fs_source = read_file(kFragmentShaderName);

  // Set shader source code
  vs_chars = vs_source.c_str();
  fs_chars = fs_source.c_str();
  vs_length = static_cast<GLint>(vs_source.length());
  fs_length = static_cast<GLint>(fs_source.length());
  glShaderSource(vs, 1, &vs_chars, &vs_length);
  glShaderSource(fs, 1, &fs_chars, &fs_length);

  // Compile shaders and chreate program
  compile_shader(vs);
  compile_shader(fs);
  prog = glCreateProgram();

  // Bind attributes
  glBindAttribLocation(prog, 0, "in_coords");
  glBindAttribLocation(prog, 1, "in_normals");

  // Attach shaders
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);

  glLinkProgram(prog);
  glUseProgram(prog);

  return prog;
}

// Read a character buffer from a file
std::string GLWidget::read_file(const char* filename) {

  // Open the file
  std::ifstream ifs(filename, std::ifstream::in);
  if(!ifs.good()) {
    std::cerr << "Couldn't find the source file " << filename << std::endl;
    exit(1);
  }

  // Read file text into string and close stream
  std::string str((std::istreambuf_iterator<char>(ifs)),
                   std::istreambuf_iterator<char>());
  ifs.close();
  return str;
}

// Compile the shader
void GLWidget::compile_shader(GLint shader) {

  GLint success;
  GLsizei log_size;
  char *log;

  // Compile shader
  glCompileShader(shader);

  // Determine if compilation completed successfully
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  // Print log if error occurred
  if (!success) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    log = new char[log_size+1];
    log[log_size] = '\0';
    glGetShaderInfoLog(shader, log_size+1, NULL, log);
    std::cout << log;
    delete(log);
    exit(1);
  }
}

// Initialize VAOs, VBOs, and IBOs
void GLWidget::init_buffers(GLuint program) {

  int loc;
  float *normal_addr;
  glm::vec3 *vertex_addr;
  unsigned short *index_addr;

  // Create a VAO for each geometry
  glGenVertexArrays(1, &vao);

  // Create two VBOs for each geometry - one for vertex positions, one for normal vector components
  glGenBuffers(2, vbos);

  // Create an IBO for each geometry
  glGenBuffers(1, &ibo);

  // Create arrays containing position and normal data
  vertex_data = new glm::vec3[kNumObjects * geom_vec[0].map["POSITION"].size/12];
  normal_data = new float[kNumObjects * geom_vec[0].map["NORMAL"].size/4];
  index_data = new unsigned short[kNumObjects * geom_vec[0].index_count];

  // Initialize array addresses
  vertex_addr = vertex_data;
  normal_addr = normal_data;
  index_addr = index_data;

  // Initialize arrays of count and indices
  count = new GLsizei[kNumObjects];
  indices = new GLvoid*[kNumObjects];

  // Concatenate arrays
  for(unsigned i=0; i<kNumObjects; i++) {

    // Set vertex positions, normal vectors, and indices
    memcpy(vertex_addr, geom_vec[0].map["POSITION"].data, geom_vec[0].map["POSITION"].size);
    memcpy(normal_addr, geom_vec[0].map["NORMAL"].data, geom_vec[0].map["NORMAL"].size);
    memcpy(index_addr, geom_vec[0].indices, geom_vec[0].index_count * sizeof(unsigned short));

    // Update the count and indices values
    count[i] = geom_vec[0].index_count;
    indices[i] = (GLvoid*)(i * geom_vec[0].index_count * sizeof(unsigned short));

    // Update addresses
    vertex_addr += geom_vec[0].map["POSITION"].size/12;
    normal_addr += geom_vec[0].map["NORMAL"].size/4;
    index_addr += geom_vec[0].index_count;
  }

  // Update vertices and indices
  for(unsigned int i=0; i<kNumObjects; i++) {

    // Update vertices
    for(unsigned j=i*num_vertices; j<(i+1)*num_vertices; j++) {
      vertex_data[j] *= sphere_vec[i].radius/0.5f;
      vertex_data[j] += sphere_vec[i].center;
    }

    for(unsigned int j=i*geom_vec[0].index_count; j<(i+1)*geom_vec[0].index_count; j++) {
      index_data[j] += i*num_vertices;
    }
  }

  // Configure VBOs to hold positions and normals for each geometry
  glBindVertexArray(vao);

  // Set vertex coordinate data
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  glBufferData(GL_ARRAY_BUFFER, kNumObjects * geom_vec[0].map["POSITION"].size,
               vertex_data, GL_STATIC_DRAW);
  loc = glGetAttribLocation(program, "in_coords");
  glVertexAttribPointer(loc, geom_vec[0].map["POSITION"].stride,
                        geom_vec[0].map["POSITION"].type, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // Set normal vector data
  glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
  glBufferData(GL_ARRAY_BUFFER, kNumObjects * geom_vec[0].map["NORMAL"].size,
               normal_data, GL_STATIC_DRAW);
  loc = glGetAttribLocation(program, "in_normals");
  glVertexAttribPointer(loc, geom_vec[0].map["NORMAL"].stride,
                        geom_vec[0].map["NORMAL"].type, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  // Set index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, kNumObjects * geom_vec[0].index_count*sizeof(unsigned short),
               index_data, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

// Initialize uniform data
void GLWidget::init_uniforms(GLuint program) {

  // Determine the locations of the color and modelview-projection matrices
  color_location = glGetUniformLocation(program, "color");
  mvp_location = glGetUniformLocation(program, "mvp");

  // Specify the modelview matrix
  modelview_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

// Initialize OpenCL processing
void GLWidget::init_cl() {

  std::string program_string;
  const char *program_chars;
  std::ostringstream motion_options, pick_options;
  char *program_log;
  size_t program_size, log_size;
  int err;

  // Identify a platform
  err = clGetPlatformIDs(1, &platform, NULL);
  if(err < 0) {
    std::cerr << "Couldn't identify a platform" << std::endl;
    exit(1);
  }

  // Access a device
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if(err == CL_DEVICE_NOT_FOUND) {
     err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  }
  if(err < 0) {
      std::cerr << "Couldn't access any devices" << std::endl;
      exit(1);
   }

  // Create OpenCL context properties
  cl_context_properties properties[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

  // Create context
  dev_context = clCreateContext(properties, 1, &device, NULL, NULL, &err);
  if(err < 0) {
    std::cerr << "Couldn't create a context" << std::endl;
    exit(1);
  }

  // Create motion program
  program_string = read_file(kMotionProgramFile);
  program_chars = program_string.c_str();
  program_size = program_string.size();
  motion_program = clCreateProgramWithSource(dev_context, 1, &program_chars, &program_size, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the program" << std::endl;
    exit(1);
  }

  // Set number of vertices
  motion_options << "-DNUM_VERTICES=" << num_vertices
                 << " -DNUM_OBJECTS=" << kNumObjects
                 << " -DVECS_PER_OBJECT=" << sizeof(SphereData)/16;

  // Build motion program
  err = clBuildProgram(motion_program, 0, NULL, motion_options.str().c_str(), NULL, NULL);
  if(err < 0) {

    // Find size of log and print to std output
    clGetProgramBuildInfo(motion_program, device, CL_PROGRAM_BUILD_LOG,
                          0, NULL, &log_size);
    program_log = new char(log_size + 1);
    program_log[log_size] = '\0';
    clGetProgramBuildInfo(motion_program, device, CL_PROGRAM_BUILD_LOG,
                          log_size + 1, (void*)program_log, NULL);
    std::cout << program_log << std::endl;
    delete(program_log);
    exit(1);
  }

  // Create pick-selection program
  program_string.clear();
  program_string = read_file(kPickSelectionProgramFile);
  program_chars = program_string.c_str();
  program_size = program_string.size();
  pick_selection_program = clCreateProgramWithSource(dev_context, 1, &program_chars, &program_size, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the program" << std::endl;
    exit(1);
  }

  // Set number of triangles for pick-selection kernel
  pick_options << "-DNUM_TRIANGLES=" << (num_triangles * kNumObjects);

  // Build pick-selection program
  err = clBuildProgram(pick_selection_program, 0, NULL, pick_options.str().c_str(), NULL, NULL);
  if(err < 0) {

    // Find size of log and print to std output
    clGetProgramBuildInfo(pick_selection_program, device, CL_PROGRAM_BUILD_LOG,
                          0, NULL, &log_size);
    program_log = new char(log_size + 1);
    program_log[log_size] = '\0';
    clGetProgramBuildInfo(pick_selection_program, device, CL_PROGRAM_BUILD_LOG,
                          log_size + 1, (void*)program_log, NULL);
    std::cout << program_log << std::endl;
    delete(program_log);
    exit(1);
  }

  // Create kernels
  collision_kernel = clCreateKernel(motion_program, kCollisionKernelName, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the collision kernel: " << err << std::endl;
    exit(1);
  };

  update_kernel = clCreateKernel(motion_program, kUpdateKernelName, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the update kernel: " << err << std::endl;
    exit(1);
  };

  motion_kernel = clCreateKernel(motion_program, kMotionKernelName, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the motion kernel: " << err << std::endl;
    exit(1);
  };

  pick_selection_kernel = clCreateKernel(pick_selection_program, kPickSelectionKernelName, &err);
  if(err < 0) {
    std::cerr << "Couldn't create the pick selection kernel: " << err << std::endl;
    exit(1);
  };

  // Determine maximum size of work groups
  clGetKernelWorkGroupInfo(update_kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
                           sizeof(obj_local_size), &obj_local_size, NULL);
  clGetKernelWorkGroupInfo(motion_kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
                           sizeof(vertex_local_size), &vertex_local_size, NULL);
  clGetKernelWorkGroupInfo(pick_selection_kernel, device, CL_KERNEL_WORK_GROUP_SIZE,
                           sizeof(pick_local_size), &pick_local_size, NULL);

  // Determine global sizes
  num_groups = (size_t)(ceil((float)kNumObjects/(float)obj_local_size));
  obj_global_size = num_groups * obj_local_size;
  num_groups = (size_t)(ceil((float)num_vertices*kNumObjects/vertex_local_size));
  vertex_global_size = num_groups * vertex_local_size;
  num_groups = (size_t)(ceil((float)num_triangles*kNumObjects/pick_local_size));
  pick_global_size = num_groups * pick_local_size;

  // Allocate memory for pick-selection result
  pick_result = new float[2*num_groups];

  // Create kernel argument from VBO
  vbo_memobj = clCreateFromGLBuffer(dev_context, CL_MEM_READ_WRITE, vbos[0], &err);
  if(err < 0) {
    std::cerr << "Couldn't create a buffer object from a VBO" << std::endl;
    exit(1);
  }

  // Create kernel argument from VBO
  ibo_memobj = clCreateFromGLBuffer(dev_context, CL_MEM_READ_WRITE, ibo, &err);
  if(err < 0) {
    std::cerr << "Couldn't create a buffer object from an IBO" << std::endl;
    exit(1);
  }

  // Create argument containing vertex data
  sphere_memobj = clCreateBuffer(dev_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                 kNumObjects * sizeof(SphereData), sphere_vec, &err);
  if(err < 0) {
    std::cerr << "Couldn't create a buffer object from a VBO" << std::endl;
    exit(1);
  }

  // Create buffer object for pick-selection results
  pick_buffer = clCreateBuffer(dev_context, CL_MEM_WRITE_ONLY, 2 * num_groups * sizeof(float), NULL, &err);
  if(err < 0) {
    std::cerr << "Couldn't create a buffer object: " << std::endl;
    exit(1);
  };

  // Make kernel arguments out of the VBO/IBO memory objects
  err = clSetKernelArg(collision_kernel, 0, sizeof(cl_mem), &sphere_memobj);
  err |= clSetKernelArg(update_kernel, 0, sizeof(cl_mem), &sphere_memobj);
  err |= clSetKernelArg(motion_kernel, 0, sizeof(cl_mem), &vbo_memobj);
  err |= clSetKernelArg(motion_kernel, 1, sizeof(cl_mem), &sphere_memobj);
  err |= clSetKernelArg(pick_selection_kernel, 0, sizeof(cl_mem), &vbo_memobj);
  err |= clSetKernelArg(pick_selection_kernel, 1, sizeof(cl_mem), &ibo_memobj);
  err |= clSetKernelArg(pick_selection_kernel, 2, sizeof(cl_mem), &pick_buffer);
  err |= clSetKernelArg(pick_selection_kernel, 3, pick_local_size*sizeof(float), NULL);
  if(err < 0) {
    std::cerr << "Couldn't set a kernel argument" << std::endl;
    exit(1);
  };

  // Create a command queue
  queue = clCreateCommandQueue(dev_context, device, 0, &err);
  if(err < 0) {
    std::cerr << "Couldn't create a command queue" << std::endl;
    exit(1);
  };
}

void GLWidget::readProperties() {

  struct SphereData select_data;
  int err;

  if(selected_object < kNumObjects && queue != NULL) {

    // Read object results
    err = clEnqueueReadBuffer(queue, sphere_memobj, CL_TRUE, selected_object * sizeof(select_data),
        sizeof(select_data), &select_data, 0, NULL, NULL);
    if(err < 0) {
      std::cerr << "Couldn't read the object information" << std::endl;
      exit(1);
    }

    win->property_browser->setSphereData(&select_data, &(sphere_props[selected_object]));
  }
}

void GLWidget::update_vertices() {

  int current_time, err;
  float delta_t;

  if(collision_kernel != NULL) {

    // Execute collision kernel
    err = clEnqueueNDRangeKernel(queue, collision_kernel, 1, NULL,
                             &obj_global_size, &obj_local_size, 0, NULL, NULL);
    if(err < 0) {
      std::cerr << "Couldn't enqueue the collision kernel" << std::endl;
      exit(1);
    }

    // Measure the elapsed time
    current_time = timer->elapsed();
    delta_t = (current_time - previous_time)/1000.0f;
    previous_time = current_time;

    // Check simulation state
    if(state == 1) {
      delta_t = 0.0f;
    }

    // Update kernel with time delta
    err = clSetKernelArg(update_kernel, 2, sizeof(float), &delta_t);
    if(err < 0) {
      std::cerr << "Couldn't set a kernel argument" << std::endl;
      exit(1);
    };

    // Execute update kernel
    err = clEnqueueNDRangeKernel(queue, update_kernel, 1, NULL, &obj_global_size,
                                 &obj_local_size, 0, NULL, NULL);
    if(err < 0) {
      std::cerr << "Couldn't enqueue the update kernel" << std::endl;
      exit(1);
    }

    glFinish();

    err = clEnqueueAcquireGLObjects(queue, 1, &vbo_memobj, 0, NULL, NULL);
    if(err < 0) {
      std::cerr << "Couldn't acquire the GL objects" << std::endl;
      exit(1);
    }

    // Execute motion kernel
    err = clEnqueueNDRangeKernel(queue, motion_kernel, 1, NULL,
        &vertex_global_size,
        &vertex_local_size, 0, NULL, NULL);
    if(err < 0) {
      std::cerr << "Couldn't enqueue the motion kernel" << std::endl;
      exit(1);
    }

    clEnqueueReleaseGLObjects(queue, 1, &vbo_memobj, 0, NULL, NULL);
    clFinish(queue);

    updateGL();
    update();
  }
}

void GLWidget::resizeGL(int width, int height) {

  int err;
  glm::vec2 dimensions;

  half_width = static_cast<float>(width)/2;
  half_height = static_cast<float>(height)/2;

  // Set window dimensions
  dimensions.x = static_cast<float>(width)/50.0f;
  dimensions.y = static_cast<float>(height)/50.0f;

  // Set new modelview matrix
  mvp_matrix = glm::ortho(0.0f, dimensions.x, 0.0f, dimensions.y, kMinZ, kMaxZ) * modelview_matrix;
  mvp_inverse = glm::inverse(mvp_matrix);
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp_matrix[0]));

  if(update_kernel != NULL) {

    // Update kernel argument
    err = clSetKernelArg(update_kernel, 1, 2*sizeof(float), glm::value_ptr(dimensions));
    if(err < 0) {
      std::cerr << "Couldn't set a kernel argument" << std::endl;
      exit(1);
    };
  }

  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

// Depict VBO data in window
void GLWidget::paintGL() {

  // Set initial color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Make sure motion kernel was created acceptably
  if(motion_kernel != NULL) {

    // Bind vertex array object
    glBindVertexArray(vao);

    // Draw objects in window
    for(unsigned i=0; i<kNumObjects; i++) {
      if(i == selected_object)
        glUniform3fv(color_location, 1, &(selected_color[0]));
      else
        glUniform3fv(color_location, 1, &(sphere_props[i].color[0]));
      glDrawElements(geom_vec[0].primitive, count[i], GL_UNSIGNED_SHORT, indices[i]);
    }

    glBindVertexArray(0);
    swapBuffers();
  }
}

QSize GLWidget::minimumSizeHint() const {
  return QSize(50, 50);
}

QSize GLWidget::sizeHint() const {
  return QSize(400, 400);
}

/*
static void qNormalizeAngle(int &angle) {
  while (angle < 0)
    angle += 360 * 16;
  while (angle > 360 * 16)
    angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle) {
  qNormalizeAngle(angle);
  if (angle != xRot) {
    xRot = angle;
    emit xRotationChanged(angle);
    updateGL();
  }
}

void GLWidget::setYRotation(int angle) {
  qNormalizeAngle(angle);
  if (angle != yRot) {
    yRot = angle;
    emit yRotationChanged(angle);
    updateGL();
  }
}

void GLWidget::setZRotation(int angle) {
  qNormalizeAngle(angle);
  if (angle != zRot) {
    zRot = angle;
    emit zRotationChanged(angle);
    updateGL();
  }
}
*/

void GLWidget::mousePressEvent(QMouseEvent *event) {

  glm::vec3 K, L, M, E, F, G, ans;
  int x, y, err;
  float t_test = 10000.0f;
  unsigned int i;

  if(event->type() == QEvent::MouseButtonPress) {

    // Compute origin (O) and direction (D) in object coordinates
    x = event->pos().x();
    y = event->pos().y();
    glm::vec4 origin = mvp_inverse * glm::vec4((x-half_width)/half_width,
        (half_height-y)/half_height, -1.0f, 1.0f);
    glm::vec4 dir = mvp_inverse * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    glm::vec4 O = glm::vec4(origin.x, origin.y, origin.z, 0.0f);
    glm::vec4 D = glm::vec4(glm::normalize(glm::vec3(dir.x, dir.y, dir.z)), 0.0f);

    // Create kernel arguments for the origin and direction
    if(pick_selection_kernel != NULL) {

      err = clSetKernelArg(pick_selection_kernel, 4, 4*sizeof(float), glm::value_ptr(O));
      err |= clSetKernelArg(pick_selection_kernel, 5, 4*sizeof(float), glm::value_ptr(D));
      if(err < 0) {
        std::cerr << "Couldn't set a kernel argument: " << err << std::endl;
        exit(1);
      };

      // Complete OpenGL processing
      glFinish();

      // Acquire lock on OpenGL objects
      err = clEnqueueAcquireGLObjects(queue, 1, &vbo_memobj, 0, NULL, NULL);
      err |= clEnqueueAcquireGLObjects(queue, 1, &ibo_memobj, 0, NULL, NULL);
      if(err < 0) {
        std::cerr << "Couldn't acquire the GL objects for pick selection" << std::endl;
        exit(1);
      }

      // Execute kernel
      err = clEnqueueNDRangeKernel(queue, pick_selection_kernel, 1, NULL, &pick_global_size,
                                   &pick_local_size, 0, NULL, NULL);
      if(err < 0) {
        std::cerr << "Couldn't enqueue the pick-selection kernel" << std::endl;
        exit(1);
      }

      // Read pick_result results
      err = clEnqueueReadBuffer(queue, pick_buffer, CL_TRUE, 0,
                                2 * num_groups * sizeof(float), pick_result, 0, NULL, NULL);
      if(err < 0) {
        std::cerr << "Couldn't read the pick-selection result buffer" << std::endl;
        exit(1);
      }

      // Deallocate and release objects
      clEnqueueReleaseGLObjects(queue, 1, &vbo_memobj, 0, NULL, NULL);
      clEnqueueReleaseGLObjects(queue, 1, &ibo_memobj, 0, NULL, NULL);

      // Check for smallest output
      for(i=0; i<2*num_groups; i+=2) {
        if(pick_result[i] < t_test) {
          t_test = pick_result[i];
         selected_object = (unsigned int)(floor((pick_result[i+1]+(i/2)*pick_local_size)/num_triangles));
        }
      }
      if(t_test == 1000) {
        selected_object = UINT_MAX;
      }
    }
  }


/*
  switch(current_tool) {
    case SELECTION:
      qDebug() << "Selection";
      break;

    case CIRCLE:
      switch(current_state) {
        case NO_CLICK:
          click_x = event->pos().x();
          click_y = event->pos().y();
          qDebug() << "State: FIRST_CLICK";
          current_state = FIRST_CLICK;
        break;

        case FIRST_CLICK:
          qDebug() << "State: NO_CLICK";
          current_state = NO_CLICK;
          win->selection_action->setChecked(true);
        break;
      }
      break;

    case RECTANGLE:
      qDebug() << "Rectangle";
      break;

    default:
      qDebug() << "Hey now";
      break;
  }
*/
}

/*
void GLWidget::mouseMoveEvent(QMouseEvent *event) {

  int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();

     if (event->buttons() & Qt::LeftButton) {
         setXRotation(xRot + 8 * dy);
         setYRotation  (yRot + 8 * dx);
     } else if (event->buttons() & Qt::RightButton) {
         setXRotation(xRot + 8 * dy);
         setZRotation(zRot + 8 * dx);
     }
     lastPos = event->pos();
}
*/


void GLWidget::dropEvent(QDropEvent* event) {

  /*
  const QMimeData *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QUrl url = mimeData->urls()[0];
    QFileInfo info(url.path());
    if(info.isFile() && info.exists() && info.suffix() == "dae") {
      qDebug() << "Got it!\n";
    }
  } else {
    qDebug() << "Can't recognize the format\n";
  }
  */
  event->accept();
}

void GLWidget::pauseSimulation() {
  state = 1;
}

void GLWidget::playSimulation() {
  state = 0;
}

void GLWidget::stopSimulation() {
  deallocateGL();

  // deallocateCL();
}

void GLWidget::dragEnterEvent(QDragEnterEvent *event) {
  event->accept();
}

void GLWidget::dragMoveEvent(QDragMoveEvent *event) {
  event->accept();
}

void GLWidget::makeSelectionActionActive() {
  current_tool = SELECTION;
}

void GLWidget::makeCircleActionActive() {
  current_tool = CIRCLE;
}

void GLWidget::makeRectActionActive() {
  current_tool = RECTANGLE;
}

/*
static void addLine(float* vertices, GLint num_verts) {
  for(int i=0; i<num_verts; i++)
    line_vertices.push_back(vertices[i]);
  line_indices.push_back(line_indices.back()+num_verts);
  line_counts.push_back(num_verts);
}
*/
