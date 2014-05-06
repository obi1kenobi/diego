#ifndef __GF_TRIANGLE_MESH_H__
#define __GF_TRIANGLE_MESH_H__

#include "IntTypes.h"
#include "Mesh.h"
#include "BBox3d.h"
#include "Vec2f.h"
#include "Vec3d.h"
#include "Vec3f.h"

#include <cassert>
#include <string>
#include <vector>

struct GfTriangle {
    GfTriangle() {}

    GfTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
        indices[0] = i0;
        indices[1] = i1;
        indices[2] = i2;
    }

    uint32_t indices[3];
};

///
/// Indexed triangle mesh abstraction.
///
/// Can contain vertices, colors and normals.
///
class GfTriangleMesh : public GfMesh
{
  public:
    /// Constructor
    GfTriangleMesh(const std::string &name);

    /// Returns \c true if the mesh is empty, i.e., with no triangles yet.
    bool IsEmpty() const {
        return _triangles.empty();
    }

    /// Set the triangles
    void SetTriangles(const std::vector<GfTriangle> &triangles) {
        _triangles = triangles;
    }

    /// Then number of indices should be a multiple of 3.
    void SetTriangles(const std::vector<uint32_t> &indices);

    /// Get triangles.
    const std::vector<GfTriangle> & GetTriangles() const {
        return _triangles;
    }

    /// Reserve triangle storage
    void ReserveTriangles(uint32_t numTriangles) {
        _triangles.reserve(numTriangles);
    }

    /// Add triangle
    uint32_t AddTriangle(uint32_t v0, uint32_t v1, uint32_t v2);

    /// Add triangle
    uint32_t AddTriangle(const GfTriangle &triangle);

    /// Get a triangle at given \p index
    const GfTriangle & GetTriangle(uint32_t index) const {
        assert(index < _triangles.size());
        return _triangles[index];
    }

    /// Update an individual triangle. It is pressumed that this triangle
    /// already exists, i.e., this will not add a triangle beyond the
    /// existing triangles in the mesh.
    void UpdateTriangle(uint32_t triIndex, uint32_t v0, uint32_t v1, uint32_t v2) {
        assert(triIndex < _triangles.size());
        _triangles[triIndex] = GfTriangle(v0, v1, v2);
    }

    /// Update an individual triangle. It is pressumed that this triangle
    /// already exists, i.e., this will not add a triangle beyond the
    /// existing triangles in the mesh.
    void UpdateTriangle(uint32_t triIndex, const GfTriangle triangle) {
        assert(triIndex < _triangles.size());
        _triangles[triIndex] = triangle;
    }

    /// Return the number of triangles.
    uint32_t GetNumTriangles() const {
        return static_cast<uint32_t>(_triangles.size());
    }

  private:
    std::vector<GfTriangle>     _triangles;
};

#endif // __GF_TRIANGLE_MESH_H__
