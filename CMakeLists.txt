cmake_minimum_required(VERSION 3.0.0)
project(SSAO VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("usr/local/include")
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL in ${OPENGL_INCLUDE_PATH}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")
# find_package(Freetype REQUIRED)
# message(STATUS "Found Freetype in ${FREETYPE_INCLUDE_DIRS}")
find_package(OpenCV REQUIRED)

set ( PRJ_INC_LIST )
set ( PRJ_SRC_LIST )
set ( PRJ_LIBRARIES )
set ( PRJ_INCLUDE_DIRS )


set ( IMGUI_HOME "${CMAKE_SOURCE_DIR}/include/imgui" )


set ( IMGUI_INC "${IMGUI_HOME}" )
set ( IMGUI_SRC "${IMGUI_HOME}" )
set ( IMGUI_BACKEND "${IMGUI_HOME}/backends" )

file ( GLOB root_c_header_files "${CMAKE_SOURCE_DIR}/*.h" )
file ( GLOB root_header_files "${CMAKE_SOURCE_DIR}/*.hpp" )
file ( GLOB root_src_files "${CMAKE_SOURCE_DIR}/*.cpp" )
file ( GLOB root_c_src_files "${CMAKE_SOURCE_DIR}/*.c" )

file ( GLOB imgui_header_files "${IMGUI_INC}/*.h" )
file ( GLOB imgui_src_files "${IMGUI_SRC}/*.cpp" )


list ( APPEND imgui_header_files "${IMGUI_BACKEND}/imgui_impl_opengl3_loader.h" )
list ( APPEND imgui_header_files "${IMGUI_BACKEND}/imgui_impl_opengl3.h" )
list ( APPEND imgui_header_files "${IMGUI_BACKEND}/imgui_impl_glfw.h" )

list ( APPEND imgui_src_files "${IMGUI_BACKEND}/imgui_impl_opengl3.cpp" )
list ( APPEND imgui_src_files "${IMGUI_BACKEND}/imgui_impl_glfw.cpp" )

list ( APPEND PRJ_INCLUDE_DIRS . )
list ( APPEND PRJ_INCLUDE_DIRS ${GLFW3_INCLUDE_DIR} )
list ( APPEND PRJ_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR} )
list ( APPEND PRJ_INCLUDE_DIRS ${IMGUI_INC} )
list ( APPEND PRJ_INCLUDE_DIRS ${IMGUI_BACKEND} )



list ( APPEND PRJ_INC_LIST ${root_header_files} )
list ( APPEND PRJ_INC_LIST ${root_c_header_files} )
list ( APPEND PRJ_INC_LIST ${imgui_header_files} )

list ( APPEND PRJ_SRC_LIST ${root_src_files} )
list ( APPEND PRJ_SRC_LIST ${imgui_src_files} )
list ( APPEND PRJ_SRC_LIST ${root_c_src_files} )

list ( APPEND PRJ_LIBRARIES ${GLFW3_LIBRARY} )
list ( APPEND PRJ_LIBRARIES ${OPENGL_LIBRARIES} )
list ( APPEND PRJ_LIBRARIES ${assimp_LIBRARIES} )
list ( APPEND PRJ_LIBRARIES ${OpenCV_LIBRARIES} )

add_executable ( ${PROJECT_NAME} ${PRJ_SRC_LIST} ${PRJ_INC_LIST}  graham.hpp)

target_include_directories ( ${PROJECT_NAME}
	PRIVATE 
        ${PRJ_INCLUDE_DIRS}
)
if(APPLE)
        target_link_libraries ( ${PROJECT_NAME} 
        PRIVATE 
                ${PRJ_LIBRARIES}
)
endif(APPLE)
if(WIN32)
        target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
        target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp )
        target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} )
endif(WIN32)
# target_link_libraries(${PROJECT_NAME} libglfw.dylib)
# target_link_libraries(${PROJECT_NAME} libassimp.dylib)
# target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})