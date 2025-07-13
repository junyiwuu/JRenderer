#Minumum cmake version
cmake_minimum_required(VERSION 3.10)

#project name and version
project(vulkanTriangle VERSION 1.0 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
#specify the c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#inlcude directories for GLFW and vulkan
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES})

#link the required libraries
#第一个参数是链接库的目标
target_link_libraries(vulkanTriangle Vulkan::Vulkan glfw)


# auto compile glsl -> spirv
add_custom_target(compile_shaders
        COMMAND chmod +x ${CMAKE_SOURCE_DIR}/compile.sh
        COMMAND ${CMAKE_SOURCE_DIR}/compile.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running compile.sh to compile shaders"
        VERBATIM)

# run compile_shaders first, before run vulkanTriangle
add_dependencies(vulkanTriangle compile_shaders)


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/tinyobjloader)
