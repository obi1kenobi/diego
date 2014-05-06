#include "TriangleMesh.h"

GfTriangleMesh::GfTriangleMesh(const std::string &name) :
    GfMesh(name)
{
}

void
GfTriangleMesh::SetTriangles(const std::vector<uint32_t> &indices)
{
    uint32_t numIndices = static_cast<uint32_t>(indices.size());
    assert(numIndices % 3 == 0);
    const uint32_t numTriangles = numIndices / 3;
    _triangles.resize(numTriangles);
    for (uint32_t i = 0; i < numTriangles; ++i) {
        uint32_t vertexIndex = i * 3;
        _triangles[i] = 
            GfTriangle(indices[vertexIndex + 0], 
                       indices[vertexIndex + 1], 
                       indices[vertexIndex + 2]);
    }
}

uint32_t
GfTriangleMesh::AddTriangle(uint32_t v0, uint32_t v1, uint32_t v2)
{
    _triangles.push_back(GfTriangle(v0, v1, v2));
    return uint32_t(_triangles.size() - 1);
}

uint32_t
GfTriangleMesh::AddTriangle(const GfTriangle &triangle)
{
    return AddTriangle(triangle.indices[0], 
                       triangle.indices[1],
                       triangle.indices[2]);
}
