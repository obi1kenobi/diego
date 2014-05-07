#ifndef __GF_MESH_H__
#define __GF_MESH_H__

#include "IntTypes.h"
#include "BBox3d.h"
#include "Vec2f.h"
#include "Vec3d.h"
#include "Vec3f.h"

#include <cassert>
#include <string>
#include <vector>

class MfMatrix4d;

///
/// Indexed mesh abstraction.
///
/// Can contain vertices, colors and normals.
///
class GfMesh
{
  public:
    enum Frequency {
        PER_VERTEX,
        PER_PRIMITIVE,
        PER_MESH,
    };

    /// Constructor
    GfMesh(const std::string &name);

    virtual ~GfMesh();

    /// Returns the name of the mesh
    const std::string & GetName() const {
        return _name;
    }

    /// Set the normal frequency, i.e., whether normals are specified
    /// per-vertex or per-primitive.
    void SetNormalFrequency(Frequency freq) {
        _normalFrequency = freq;
    }

    /// Get the normal frequency, i.e., whether normals are specified
    /// per-vertex or per-primitive.
    Frequency GetNormalFrequency() const {
        return _normalFrequency;
    }

    /// Set the color frequency, i.e., whether colors are specified
    /// per-vertex or per-primitive.
    void SetColorFrequency(Frequency freq) {
        _colorFrequency = freq;
    }

    /// Get the color frequency, i.e., whether colors are specified
    /// per-vertex or per-primitive.
    Frequency GetColorFrequency() const {
        return _colorFrequency;
    }

    /// Set the mesh vertices
    void SetVertices(const std::vector<MfVec3d> &vertices) {
        _vertices = vertices;
    }

    /// Get the mesh vertices. Can be empty.
    const std::vector<MfVec3d> & GetVertices() const {
        return _vertices;
    }

    /// Reserve storage for vertices
    void ReserveVertices(uint32_t numVertices) {
        _vertices.reserve(numVertices);
    }

    /// Add vertex
    void AddVertex(const MfVec3d &vertex) {
        _vertices.push_back(vertex);
    }

    /// Update an individual vertex. It is pressumed that this vertex
    /// already exists, i.e., this will not add a vertex beyond the
    /// existing vertices in the mesh.
    void UpdateVertex(uint32_t index, const MfVec3d &vertex) {
        assert(index < _vertices.size());
        _vertices[index] = vertex;
    }

    /// Get vertex at given \p index.
    const MfVec3d & GetVertex(uint32_t index) const {
        assert(index < _vertices.size());
        return _vertices[index];
    }

    /// Get the number of vertices
    uint32_t GetNumVertices() const {
        return static_cast<uint32_t>(_vertices.size());
    }

    /// Set the normals.
    void SetNormals(const std::vector<MfVec3d> &normals) {
        _normals = normals;
    }

    /// Get the normals. Can be empty.
    const std::vector<MfVec3d> & GetNormals() const {
        return _normals;
    }

    /// Reserve storage for normals
    void ReserveNormals(uint32_t numNormals) {
        _normals.reserve(numNormals);
    }

    /// Add normal
    void AddNormal(const MfVec3d &normal) {
        _normals.push_back(normal);
    }

    /// Update an individual normal. It is pressumed that this normal
    /// already exists, i.e., this will not add a normal beyond the
    /// existing normals in the mesh.
    void UpdateNormal(uint32_t index, const MfVec3d &normal) {
        assert(index < _normals.size());
        _normals[index] = normal;
    }

    /// Get normal at given \p index.
    const MfVec3d & GetNormal(uint32_t index) const {
        assert(index < _normals.size());
        return _normals[index];
    }

    /// Get the number of normals
    uint32_t GetNumNormals() const {
        return static_cast<uint32_t>(_normals.size());
    }

    /// Set the colors. 
    void SetColors(const std::vector<MfVec3f> &colors) {
        _colors = colors;
    }

    /// Get the colors. Can be empty.
    const std::vector<MfVec3f> & GetColors() const {
        return _colors;
    }

    /// Reserve storage for colors
    void ReserveColors(uint32_t numColors) {
        _colors.reserve(numColors);
    }

    /// Add color
    void AddColor(const MfVec3f &color) {
        _colors.push_back(color);
    }

    /// Update an individual color. It is pressumed that this color
    /// already exists, i.e., this will not add a color beyond the
    /// existing colors in the mesh.
    void UpdateColor(uint32_t index, const MfVec3f &color) {
        assert(index < _colors.size());
        _colors[index] = color;
    }

    /// Get color at given \p index.
    const MfVec3f & GetColor(uint32_t index) const {
        assert(index < _colors.size());
        return _colors[index];
    }

    /// Get the number of colors
    uint32_t GetNumColors() const {
        return static_cast<uint32_t>(_colors.size());
    }

    /// Set the texture coordinates. 
    void SetTextureCoordinates(const std::vector<MfVec2f> &texCoords) {
        _texCoords = texCoords;
    }

    /// Get the texture coordinates. Can be empty.
    const std::vector<MfVec2f> & GetTextureCoordinates() const {
        return _texCoords;
    }

    /// Reserve storage for texture coordinates
    void ReserveTextureCoordinates(uint32_t numTexCoords) {
        _texCoords.reserve(numTexCoords);
    }

    /// Add texture coordinate
    void AddTextureCoordinate(const MfVec2f &texCoord) {
        _texCoords.push_back(texCoord);
    }

    /// Update an individual texture coordinate. It is pressumed that 
    /// this texture coordinate already exists, i.e., this will not add 
    /// a texture coordinate beyond the existing texture coordinate in 
    /// the mesh.
    void UpdateTextureCoordinate(uint32_t index, const MfVec2f &texCoord) {
        assert(index < _texCoords.size());
        _texCoords[index] = texCoord;
    }

    /// Get texture coordinate at given \p index.
    const MfVec2f & GetTextureCoordinate(uint32_t index) const {
        assert(index < _texCoords.size());
        return _texCoords[index];
    }

    /// Get the number of texture coordinates
    uint32_t GetNumTextureCoordinates() const {
        return static_cast<uint32_t>(_texCoords.size());
    }

    /// Calculate the bounding box of this mesh
    MfBBox3d CalculateBBox() const;

    /// Transforms the coordinates with the given transformation matrix.
    void Transform(const MfMatrix4d &mat);

  private:
    std::string                 _name;
    Frequency                   _normalFrequency;
    Frequency                   _colorFrequency;
    std::vector<MfVec3d>        _vertices;
    std::vector<MfVec3d>        _normals;
    std::vector<MfVec3f>        _colors;
    std::vector<MfVec2f>        _texCoords;
};

#endif // __GF_MESH_H__
