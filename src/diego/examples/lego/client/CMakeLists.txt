cmake_minimum_required(VERSION 2.6)

project(LegoClient)

set(TARGET "LegoClient")

set(CPP_FILES
    BBox3d.cpp
    LegoBrick.cpp
    LegoUniverse.cpp
    LegoOps.cpp
    LegoTransactionMgr.cpp
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
    main.cpp
)

add_definitions("-std=c++11")
add_executable(${TARGET} ${CPP_FILES})
