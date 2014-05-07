cmake_minimum_required(VERSION 2.8.11)

project(Lego)

set(BIN_NAME LegoClient)

set(LIBRARY_CPP_FILES
    BBox3d.cpp
    LegoApp.cpp
    LegoBrick.cpp
    LegoMainWindow.cpp
    LegoOps.cpp
    LegoTransaction.cpp
    LegoTransactionMgr.cpp
    LegoUniverse.cpp
    Matrix4d.cpp
    Mesh.cpp
    STLWriter.cpp
    TriBox3.cpp
    TriangleMesh.cpp
    Vec2d.cpp
    Vec2f.cpp
    Vec2i.cpp
    Vec3d.cpp
    Vec3f.cpp
    Vec3i.cpp
    Vec4d.cpp
    Vec4f.cpp
)

set(BIN_CPP_FILES
    main.cpp
   )

set(UI_FILES
    forms/MainWindow.ui
   )

set(TEST_CPP_FILES
    TestClient.cpp
   )

set(LIBS_USED
    Coin
    SoQt
   )

# XXX: Need to find a better way to do this
include_directories(${COIN_DIR}/include)
include_directories(${SOQT_DIR}/include)
link_directories(${COIN_DIR}/lib)
link_directories(${SOQT_DIR}/lib)

add_definitions("-std=c++11")

# Prep Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)
include_directories("${Qt5Core_INCLUDE_DIRS}/..")
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# Library
set(LIB_NAME "LegoClientFramework")
qt5_wrap_ui(UI_OUT_FILES ${UI_FILES})
add_library(${LIB_NAME} ${LIBRARY_CPP_FILES} ${UI_OUT_FILES})

# Client app
add_executable(${BIN_NAME} ${BIN_CPP_FILES})
target_link_libraries(${BIN_NAME} ${LIB_NAME})
target_link_libraries(${BIN_NAME} ${LIBS_USED})
target_link_libraries(${BIN_NAME} Qt5::Widgets) 
target_link_libraries(${BIN_NAME} ${OPENGL_LIBRARIES})

# Test apps
set(TEST_NAME "LegoTest")
add_executable(${TEST_NAME} ${TEST_CPP_FILES})
target_link_libraries(${TEST_NAME} ${LIB_NAME})
