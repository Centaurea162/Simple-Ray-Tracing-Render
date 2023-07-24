cmake_minimum_required(VERSION 3.16)
project(RayTracing C CXX)
set(CMAKE_CXX_STANDARD 11)

set(INC_DIR
        ./include
        )

##声明链接库路径
#set(LINK_DIR
#        ./Dependencies/glfw/lib-mingw-w64
#        ./Dependencies/glew/lib/Release/x64
#        )
#
include_directories(${INC_DIR})
#link_directories(${LINK_DIR})


add_executable(RayTracing src/main.cpp)

#target_link_libraries(OpenGL glfw3 opengl32 glew32s )