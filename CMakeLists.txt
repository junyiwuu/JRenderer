#Minumum cmake version
cmake_minimum_required(VERSION 3.10)

#project name and version
project(vulkanTriangle VERSION 1.0 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
#specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

#inlcude directories for GLFW and vulkan
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/Engine/*.cpp" "${CMAKE_SOURCE_DIR}/Engine/*.hpp")
file(GLOB IMGUI_SRC
        third_party/imgui/imgui*.cpp
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_vulkan.cpp)
add_library(imgui ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
                                ${CMAKE_SOURCE_DIR}/third_party/imgui
                                ${CMAKE_SOURCE_DIR}/third_party/imgui/backends)
target_link_libraries(imgui PUBLIC 
                                glfw
                                Vulkan::Vulkan )


add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")


#link the required libraries
#第一个参数是链接库的目标
target_link_libraries(vulkanTriangle Vulkan::Vulkan glfw imgui)


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


