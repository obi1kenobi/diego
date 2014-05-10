#include "STLReader.h"

#include "TriangleMesh.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

GfSTLReader::GfSTLReader()
{
}

GfTriangleMesh *
GfSTLReader::Read(const std::string &fileName)
{
    // See if ASCII or binary
    std::ifstream stl(fileName.c_str(), 
                      std::ifstream::in | std::ifstream::binary | std::ifstream::ate);
    std::streampos fileSize = stl.tellg();
    stl.seekg(0, std::ifstream::beg);
    const int headerSize = 80;
    char buffer[headerSize];
    stl.read(buffer, headerSize);
    uint32_t numTriangles = 0;
    stl.read(reinterpret_cast<char *>(&numTriangles), sizeof(numTriangles));
    if (!stl.good()) {
        std::cerr << "ERROR: Bad STL file " << fileName << std::endl;
        return NULL;
    }
    stl.close();

    // A binary file consists of an 80 byte header, a 4 byte (number of
    // triangles) and then the triangles (50 bytes each)
    const int triangleSize = 50;
    if (fileSize == headerSize + sizeof(numTriangles) + numTriangles * triangleSize) {
        return _ReadBinary(fileName);
    } else {
        return _ReadASCII(fileName);
    }
}

GfTriangleMesh *
GfSTLReader::_ReadASCII(const std::string &fileName)
{
    std::ifstream stl(fileName.c_str(), std::ifstream::in);

    // Read mesh type
    std::string buffer;
    stl >> buffer;
    if (buffer != "solid") {
        std::cerr << "The STL file does not begin with \"solid\"" << std::endl;
        return NULL;
    }

    // Read name
    char nameBuffer[1024];
    stl.getline(nameBuffer, sizeof(nameBuffer));
    if (stl.fail()) {
        std::cerr << "Invalid name of STL mesh on first line.\n" << std::endl;
            return NULL;
    }

    std::vector<MfVec3d> vertices;
    std::vector<MfVec3d> normals;
    std::vector<uint32_t> triangles;

    uint32_t numVertices = 0;
    while (stl.good()) {
        stl >> buffer;
        if (buffer == "endsolid")
            break;
        else if (buffer != "facet") {
            _ReportExpectedError("facet", buffer);
            return NULL;
        }

        if (!_ConsumeToken(stl, "normal")) return NULL;
        MfVec3d normal = _ReadVec3d(stl);

        if (!_ConsumeToken(stl, "outer")) return NULL;
        if (!_ConsumeToken(stl, "loop")) return NULL;

        if (!_ConsumeToken(stl, "vertex")) return NULL;
        MfVec3d v1 = _ReadVec3d(stl);

        if (!_ConsumeToken(stl, "vertex")) return NULL;
        MfVec3d v2 = _ReadVec3d(stl);

        if (!_ConsumeToken(stl, "vertex")) return NULL;
        MfVec3d v3 = _ReadVec3d(stl);

        if (!_ConsumeToken(stl, "endloop")) return NULL;
        if (!_ConsumeToken(stl, "endfacet")) return NULL;

        if (v1 == v2 || v1 == v3 || v2 == v3) {
            std::cerr 
                << "ERROR: Degenerate triangle #" << triangles.size() 
                << " with vertices: " << v1 << ", " << v2 << ", " << v3 
                << std::endl;
        }

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
        triangles.push_back(numVertices++);
        triangles.push_back(numVertices++);
        triangles.push_back(numVertices++);
    }

    GfTriangleMesh *mesh = new GfTriangleMesh(buffer);
    mesh->SetVertices(vertices);
    mesh->SetNormals(normals);
    mesh->SetTriangles(triangles);

    return mesh;
}

GfTriangleMesh *
GfSTLReader::_ReadBinary(const std::string &fileName)
{
    std::ifstream stl(fileName.c_str(), std::ifstream::in | std::ifstream::binary);

    // Skip header
    stl.seekg(80);
    uint32_t numTriangles;
    stl.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));

    std::vector<MfVec3d> vertices;
    std::vector<MfVec3d> normals;
    std::vector<uint32_t> triangles;

    vertices.reserve(numTriangles * 3);
    normals.reserve(numTriangles * 3);
    triangles.reserve(numTriangles);

    uint32_t numVertices = 0;
    for (uint32_t i = 0; i < numTriangles; ++i) {
        if (stl.bad()) {
            std::cerr << "Reached end of file without reading all triangles" << std::endl;
        }
        MfVec3d normal = _ReadVec3dBinary(stl);
        MfVec3d v1 = _ReadVec3dBinary(stl);
        MfVec3d v2 = _ReadVec3dBinary(stl);
        MfVec3d v3 = _ReadVec3dBinary(stl);

        if (v1 == v2 || v1 == v3 || v2 == v3) {
            std::cerr 
                << "ERROR: Degenerate triangle #" << triangles.size() 
                << " with vertices: " << v1 << ", " << v2 << ", " << v3 
                << std::endl;
        }

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
        triangles.push_back(numVertices++);
        triangles.push_back(numVertices++);
        triangles.push_back(numVertices++);

        // Skip 2 unused bytes
        stl.seekg(2, std::ios::cur);
    }

    GfTriangleMesh *mesh = new GfTriangleMesh(fileName);
    mesh->SetVertices(vertices);
    mesh->SetNormals(normals);
    mesh->SetTriangles(triangles);

    return mesh;
}

bool
GfSTLReader::_ConsumeToken(std::ifstream &is, const std::string &token)
{
    std::string buffer;
    is >> buffer;
    if (buffer != token) {
        _ReportExpectedError(token, buffer);
        return false;
    } else {
        return true;
    }
}

MfVec3d
GfSTLReader::_ReadVec3d(std::ifstream &is)
{
    float vec[3];
    is >> vec[0];
    is >> vec[1];
    is >> vec[2];
    return MfVec3d(vec[0], vec[1], vec[2]);
}

MfVec3d
GfSTLReader::_ReadVec3dBinary(std::ifstream &is)
{
    float vec[3];
    for (int i = 0; i < 3; ++i) {
        is.read(reinterpret_cast<char*>(&vec[i]), sizeof(vec[i]));
    }
    return MfVec3d(vec[0], vec[1], vec[2]);
}

void
GfSTLReader::_ReportExpectedError(const std::string &expected,
                                  const std::string &found)
{
    std::cerr 
        << "Expected keyword \"" 
        << expected << "\". Found \"" 
        << found << "\"" << std::endl;
}
