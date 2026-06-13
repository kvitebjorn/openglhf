/* GLAD must be included before GLFW */
// GLAD loads the OpenGL functions
// GLFW handles window creation by creating the OpenGL context
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/**
 * @brief Utility function to load shader source code into a string.
 *
 * @param filePath The path to the shader source file.
 * @return string A string containing the shader source code.
 */
string readFile(const string &filePath)
{
  ifstream file(filePath);
  if (!file.is_open())
  {
    cerr << "ERROR: Could not open shader file: " << filePath << endl;
    return "";
  }

  stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}

/**
 * @brief The main window resize handler; updates the viewport to match the new window dimensions.
 * The type signature matches that expected by the GLFW callback registration mechanism.
 *
 * @param window The GLFW window that just got resized.
 * @param width The new width of the window.
 * @param height The new height of the window.
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

/**
 * @brief Handle inputs.
 *
 * @param window The GLFW window to check for input signals on.
 */
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

/**
 * @brief The main render loop. Each iteration is coloquially known as a "frame".
 *
 * @param window The GLFW window context to render to.
 */
void render(GLFWwindow *window)
{
  /* set up vertex data and buffers */
  float vertices[] = {
      -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
      0.0f, 0.5f, 0.0f};
  unsigned int VBO;
  glGenBuffers(1, &VBO);                                                     // generates a name for all our buffers (in this case just 1)
  glBindBuffer(GL_ARRAY_BUFFER, VBO);                                        // bind to the buffer binding point on the gpu to prep for vertex shader
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy cpu data into the gpu buffer!

  // TODO: make a new shader program func
  // TODO: make this a generic func (shader type, path) and include comp and err handling
  /* OpenGL mandates we provide at least a vertex & fragment shader ... */
  /* compile the vertex shader */
  string vertexShaderSource = readFile("shaders/shader.vert");
  const char *vertexShaderSource_ptr = vertexShaderSource.data();
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource_ptr, NULL);
  glCompileShader(vertexShader);

  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED"
         << endl
         << vertexShaderSource
         << endl
         << infoLog
         << endl;
    exit(-1);
  }

  /* compile the fragment shader */
  unsigned int fragmentShader;
  string fragmentShaderSource = readFile("shaders/shader.frag");
  const char *fragmentShaderSource_ptr = fragmentShaderSource.data();
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource_ptr, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED"
         << endl
         << fragmentShaderSource
         << endl
         << infoLog
         << endl;
    exit(-1);
  }

  /* link the shaders */
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINK_FAILED"
         << endl
         << infoLog
         << endl;
    exit(-1);
  }

  /* activate our shader program! */
  glUseProgram(shaderProgram);

  /* immediately delete shaders, we don't need them anymore! */
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // the main render loop
  while (!glfwWindowShouldClose(window))
  {
    /* handle input */
    processInput(window);

    /* rendering commands - draw to the "back buffer" of our double buffer */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // configure how colors will get cleared "state-setting fn"
    glClear(GL_COLOR_BUFFER_BIT);         // clear colors "state-using fn"

    /* finalize rendering - swap the "back buffer" to be the front-facing one! */
    glfwSwapBuffers(window);

    /* queue up any input or window events for next iteration, and run callbacks */
    glfwPollEvents();
  }
}

/**
 * @brief Program entry point.
 *
 * @return int A status code indicating success.
 */
int main(void)
{
  /* initialize */
  if (!glfwInit())
  {
    cout << "Failed to initialize GLFW" << endl;
    return -1;
  }

  // a slight nuance in order: GLAD to get initialized later

  /* configure */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  /* setup */
  GLFWwindow *window = glfwCreateWindow(800, 600, "opengl hf", NULL, NULL);
  if (window == NULL)
  {
    cout << "Failed to create GLFW window" << endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // we have to defer GLAD's initialization until after we get an OpenGL context from GLFW
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }

  // make sure GLAD is initialized before calling any `gl` functions!!!
  glViewport(0, 0, 800, 600);

  /* register callbacks */
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  /* main render loop */
  render(window);

  /* cleanup */
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
