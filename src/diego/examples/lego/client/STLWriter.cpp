#include "STLWriter.h"

#include "TriangleMesh.h"
#include "Vec3f.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <assert.h>

GfSTLWriter::GfSTLWriter()
{
}

void
GfSTLWriter::Write(const std::string &fileName, 
                   const GfTriangleMesh &mesh,
                   Format format)
{
    if (format == ASCII) {
        _WriteASCII(fileName, mesh);
    } else {
        _WriteBinary(fileName, mesh);
    }
}

void
GfSTLWriter::_WriteASCII(const std::string &fileName, 
                         const GfTriangleMesh &mesh)
{
    const std::vector<MfVec3d> &vertices = mesh.GetVertices();
    const std::vector<MfVec3d> &normals = mesh.GetNormals();
    const std::vector<GfTriangle> &triangles = mesh.GetTriangles();

    // Only support meshes with normal per triangle for now
    if (!normals.empty()) {
        assert(mesh.GetNormalFrequency() == GfTriangleMesh::PER_PRIMITIVE);
        assert(normals.size() == triangles.size());
    }

    std::ofstream os(fileName.c_str());
    if (os.bad()) {
        std::cerr << "Error writing out " << fileName << std::endl;
        return;
    }

    os << "solid " << mesh.GetName() << std::endl;
    size_t numTriangles = triangles.size();
    for (size_t t = 0; t < numTriangles; ++t) {
        const GfTriangle &tri = triangles[t];

        os << "facet normal ";
        if (normals.empty()) {
            _WriteVec3d(os, MfVec3d(0, 0, 0));
        } else {
            _WriteVec3d(os, normals[t]);
        }
        os << std::endl;
        os << " outer loop" << std::endl;
        for (size_t v = 0; v < 3; ++v) {
            os << "  vertex "; 
            _WriteVec3d(os, vertices[tri.indices[v]]);
            os << std::endl;
        }
        os << " endloop" << std::endl;
        os << "endfacet" << std::endl;
    }

    os << "endsolid " << mesh.GetName() << std::endl;
}

void
GfSTLWriter::_WriteBinary(const std::string &fileName, 
                          const GfTriangleMesh &mesh)
{
    const std::vector<MfVec3d> &vertices = mesh.GetVertices();
    const std::vector<MfVec3d> &normals = mesh.GetNormals();
    const std::vector<GfTriangle> &triangles = mesh.GetTriangles();

    // Only support meshes with normal per triangle for now
    if (!normals.empty()) {
        assert(mesh.GetNormalFrequency() == GfTriangleMesh::PER_PRIMITIVE);
        assert(normals.size() == triangles.size());
    }

    std::ofstream os(fileName.c_str(), std::ios::binary);
    if (os.bad()) {
        std::cerr << "Error writing out " << fileName << std::endl;
        return;
    }

    // Empty header
    char header[80];
    std::memset(&header, 0, sizeof(header));
    os.write(header, sizeof(header));

    uint32_t numTriangles = uint32_t(mesh.GetNumTriangles());
    os.write(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));

    for (uint32_t t = 0; t < numTriangles; ++t) {
        const GfTriangle &tri = triangles[t];
        if (normals.empty()) {
            _WriteVec3dBinary(os, MfVec3d(0, 0, 0));
        } else {
            _WriteVec3dBinary(os, normals[t]);
        }
        for (int v = 0; v < 3; ++v) {
            _WriteVec3dBinary(os, vertices[tri.indices[v]]);
        }
        // Unused two bytes
        os.put(0);
        os.put(0);
    }
}

void
GfSTLWriter::_WriteVec3d(std::ofstream &os, const MfVec3d &v)
{
    // Write out as a 32-bit floating point
    MfVec3f vec(v);
    os << vec[0] << " " << vec[1] << " " << vec[2];
}

void
GfSTLWriter::_WriteVec3dBinary(std::ofstream &os, const MfVec3d &v)
{
    // Write out as a 32-bit floating point
    MfVec3f vec(v);
    for (int i = 0; i < 3; ++i) {
        os.write(reinterpret_cast<const char*>(&vec[i]), sizeof(vec[i]));
    }
}
