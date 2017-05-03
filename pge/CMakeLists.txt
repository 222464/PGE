cmake_minimum_required(VERSION 3.1)

project(PGE)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE SOURCES
    "source/*.h"
    "source/*.cpp"
)
list(APPEND SOURCES "source/gl_core_4_4.c")
 
add_executable(pge ${SOURCES})
include_directories("${PROJECT_SOURCE_DIR}/source")
#target_compile_definitions(pge PRIVATE PGE_DEBUG)

find_package(SFML 2 REQUIRED system window graphics network audio)
message(STATUS "SFML incs: " ${SFML_INCLUDE_DIR})
message(STATUS "SFML libs: " ${SFML_LIBRARIES})
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(pge ${SFML_LIBRARIES})

find_package(OpenGL REQUIRED)
message(STATUS "OpenGL incs: " ${OPENGL_INCLUDE_DIR})
message(STATUS "OpenGL libs: " ${OPENGL_LIBRARIES})
if(OPENGL_GLU_FOUND)
  message(STATUS "Found GLU")
endif()
if(OPENGL_XMESA_FOUND)
  message(STATUS "Found XMesa")
endif()
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(pge ${OPENGL_LIBRARIES})

find_package(Bullet REQUIRED)
message(STATUS "Bullet incs: " ${BULLET_INCLUDE_DIR})
message(STATUS "Bullet libs: " ${BULLET_LIBRARIES})
include_directories(${BULLET_INCLUDE_DIR})
target_link_libraries(pge ${BULLET_LIBRARIES})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC pge "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(pge "${CMAKE_THREAD_LIBS_INIT}")
endif()

if(MSVC)
  set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
     )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
endif()

set_property(TARGET pge PROPERTY CXX_STANDARD 11)
set_property(TARGET pge PROPERTY CXX_STANDARD_REQUIRED ON)

