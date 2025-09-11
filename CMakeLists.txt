cmake_minimum_required(VERSION 3.10)
project(JRenderer VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include(FetchContent)



find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)
# set(Ktx_DIR "/usr/local/lib/cmake/ktx" CACHE PATH "Path to ktxConfig.cmake directory")


find_package(Ktx CONFIG QUIET) # 会找 ktxConfig.cmake。 QUITE会找不到就进入ifelse branch，如果改成required会直接报错
# 如果系统没找到，就用子模块
if(TARGET KTX::ktx)
  set(KTX_TARGET KTX::ktx)
else()
  message(STATUS "Using bundled KTX-Software (third_party).")
  set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
  set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
  set(KTX_TARGET ktx)

  add_subdirectory(third_party/KTX-Software EXCLUDE_FROM_ALL) #将这个ktx-software作为子目录加进来。 excludefromall意思是所有target得显式要求才会编
endif()



find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
  message(STATUS "Fetching GLFW (no system install found)")
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
endif()


# GLM (header-only)
find_package(glm QUIET)
if(NOT glm_FOUND)
  message(STATUS "Fetching GLM")
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
  )
  FetchContent_MakeAvailable(glm)
endif()


# Assimp
find_package(assimp QUIET)
if(NOT assimp_FOUND)
  message(STATUS "Fetching Assimp")
  FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v6.0.2
  )
  set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(assimp)
  # target 'assimp' provided
endif()



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

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Vulkan::Vulkan
    glfw
    imgui
    assimp
    glm::glm
    ${KTX_TARGET}
    ${X11_LIBRARIES}
)




set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")




# auto compile glsl -> spirv
add_custom_target(compile_shaders
        COMMAND chmod +x ${CMAKE_SOURCE_DIR}/compile.sh
        COMMAND ${CMAKE_SOURCE_DIR}/compile.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running compile.sh to compile shaders"
        VERBATIM)

# run compile_shaders first, before run JRenderer
add_dependencies(JRenderer compile_shaders)


# USE: 
# Debug build (with sanitizers):
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# make

# Release build (optimized, no sanitizers):
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make

