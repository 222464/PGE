cmake_minimum_required(VERSION 2.8)

project(PGE)

include_directories("${PROJECT_SOURCE_DIR}/source")

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2 REQUIRED system window graphics network audio)
 
include_directories(${SFML_INCLUDE_DIR})

find_package(GLEW REQUIRED)
 
include_directories(${GLEW_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
 
include_directories(${OPENGL_INCLUDE_DIR})

find_package(Bullet REQUIRED)
 
include_directories(${BULLET_INCLUDE_DIR})

file(GLOB_RECURSE SOURCES
    "source/*.h"
	"source/*.cpp"
)

#add_library(d3d-static STATIC ${SOURCES})
#add_library(d3d-shared SHARED ${SOURCES})

#target_link_libraries(d3d-static ${SFML_LIBRARIES})
#target_link_libraries(d3d-static ${GLEW_LIBRARIES})
#target_link_libraries(d3d-static ${OPENGL_LIBRARIES})
#target_link_libraries(d3d-static ${BULLET_LIBRARIES})

#target_link_libraries(d3d-shared ${GLEW_LIBRARIES})
#target_link_libraries(d3d-shared ${SFML_LIBRARIES})
#target_link_libraries(d3d-shared ${OPENGL_LIBRARIES})
#target_link_libraries(d3d-shared ${BULLET_LIBRARIES})

add_executable(pge ${SOURCES})

target_link_libraries(pge ${GLEW_LIBRARIES})
target_link_libraries(pge ${SFML_LIBRARIES})
target_link_libraries(pge ${OPENGL_LIBRARIES})
target_link_libraries(pge ${BULLET_LIBRARIES})
