#include "Mesh.h"

#include "Matrix4d.h"

GfMesh::GfMesh(const std::string &name) :
    _name(name),
    _normalFrequency(PER_VERTEX),
    _colorFrequency(PER_VERTEX)
{
}

GfMesh::~GfMesh()
{
}

MfBBox3d
GfMesh::CalculateBBox() const
{
    MfBBox3d bbox;
    for (const auto &vertex : _vertices) {
        bbox.ExtendBy(vertex);
    }
    return bbox;
}

void
GfMesh::Transform(const MfMatrix4d &mat)
{
    for (auto &vertex : _vertices) {
        vertex = mat.Transform(vertex);
    }
}
