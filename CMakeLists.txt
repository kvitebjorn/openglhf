cmake_minimum_required(VERSION 3.14)
project(openglhf LANGUAGES C CXX) # C is for glad.c

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# locate external dependencies
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# manual library definitions
# check out https://glad.dav1d.de/ to generate a GLAD loader!
add_library(glad STATIC 
    /home/kyle/dev/third_party/glad/src/glad.c
)
target_include_directories(glad PUBLIC 
    /home/kyle/dev/third_party/glad/include
)

# executable target definition
add_executable(openglhf src/main.cpp)

# link everything together 
target_link_libraries(openglhf PRIVATE 
    glad 
    glm::glm 
    glfw 
    OpenGL::GL
)
