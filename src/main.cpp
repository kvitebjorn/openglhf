/* GLAD must be included before GLFW */
// GLAD loads the OpenGL functions
// GLFW handles window creation by creating the OpenGL context
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

const string VERTEX_TYPE_STR = "VERTEX";
const string FRAGMENT_TYPE_STR = "FRAGMENT";
const unordered_map<GLenum, string> SHADER_TYPE_TO_STR = {
    {GL_VERTEX_SHADER, VERTEX_TYPE_STR},
    {GL_FRAGMENT_SHADER, FRAGMENT_TYPE_STR}};

struct OpenGLMesh
{
  unsigned int vao = 0;
  unsigned int vbo = 0;

  ~OpenGLMesh()
  {
    cleanup();
  }

  void cleanup()
  {
    if (vbo != 0)
    {
      glDeleteBuffers(1, &vbo);
      vbo = 0;
    }
    if (vao != 0)
    {
      glDeleteVertexArrays(1, &vao);
      vao = 0;
    }
  }
};

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
void framebuffer_size_callback(GLFWwindow *window, const int width, const int height)
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
 * @brief Compiles a shader given its type and path to the shader definition.
 *
 * @param shaderType The type of shader to compile.
 * @param path The path to the shader definition file.
 * @return unsigned int The id of the compiled shader.
 */
unsigned int compileShader(GLenum shaderType, const string path)
{
  const string shaderSource = readFile(path);
  const char *shaderSource_ptr = shaderSource.data();
  unsigned int shader;
  shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource_ptr, NULL);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::"
         << SHADER_TYPE_TO_STR.at(shaderType)
         << "::COMPILATION_FAILED"
         << endl
         << shaderSource
         << endl
         << infoLog
         << endl;
    exit(-1);
  }

  return shader;
}

/**
 * @brief Create a shader program object from hardcoded shader paths.
 *
 * @return unsigned int The id of the shader program.
 */
unsigned int createShaderProgram()
{
  /* OpenGL mandates we provide at least a vertex & fragment shader ... */
  const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, "../shaders/shader.vert");
  const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, "../shaders/shader.frag");

  /* link the shaders */
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  int success;
  char infoLog[512];
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

  /* immediately delete shaders, we don't need them anymore! */
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

/**
 * @brief Creates a mesh, which is conceptually both a VBO and a VAO for raw triangle vertex data.
 *
 * @param vertices The triangle's vertices.
 * @return OpenGLMesh A struct that tracks the id of both the VAO and VBO that were created.
 */
OpenGLMesh createOpenGLMesh(const std::vector<float> &vertices)
{
  /* conceptually link the vertex data to the vertex shader -
     in other words, describes how the VBO should be understand by the vertex shader.
     a VBO is a raw buffer on gpu that contains our vertices.
     a VAO is basically a lens that tells how the data (VBO) is shaped and where it starts. */
  OpenGLMesh mesh;

  /* set up the VAO */
  glGenVertexArrays(1, &mesh.vao); // generates a name for our vertex array obj
  glBindVertexArray(mesh.vao);     // binds a vertex array obj to the generated name above

  /* set up our VBO (the VAO will track this...) */
  glGenBuffers(1, &mesh.vbo);                                                                      // generates a name for all our buffers (in this case just 1)
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);                                                         // bind to the buffer binding point on the gpu to prep for vertex shader
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW); // copy cpu data into the gpu buffer!

  /* configure the layout on our VAO */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0); // see `shader.vert` - notice the `location = 0`? ;)

  /* unbind VAO ... just in case ;) */
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return mesh;
}

/**
 * @brief The main render loop. Each iteration is coloquially known as a "frame".
 *
 * @param window The GLFW window context to render to.
 */
void render(GLFWwindow *window)
{
  /* our raw triangle vertex data */
  const std::vector<float> vertices = {
      -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
      0.0f, 0.5f, 0.0f};

  /* compile and use our shaders! */
  const unsigned int shaderProgram = createShaderProgram();
  glUseProgram(shaderProgram);

  /* get our triangle ready! */
  const OpenGLMesh triangle = createOpenGLMesh(vertices);

  // the main render loop
  while (!glfwWindowShouldClose(window))
  {
    /* handle input */
    processInput(window);

    /* rendering commands - draw to the "back buffer" of our double buffer */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // configure how colors will get cleared "state-setting fn"
    glClear(GL_COLOR_BUFFER_BIT);         // clear colors "state-using fn"

    /* draw our beautiful triangle */
    glUseProgram(shaderProgram);
    glBindVertexArray(triangle.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

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
