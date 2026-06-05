/* GLAD must be included before GLFW */
// GLAD loads the OpenGL functions
// GLFW handles window creation by creating the OpenGL context
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream> // TODO: logger instead

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
    std::cout << "Failed to initialize GLFW" << std::endl;
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
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // we have to defer GLAD's initialization until after we get an OpenGL context from GLFW
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
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
