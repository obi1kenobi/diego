#ifndef __GF_STL_WRITER_H__
#define __GF_STL_WRITER_H__

#include <string>

class GfTriangleMesh;
class MfVec3d;

class GfSTLWriter
{
  public:
    enum Format {
        ASCII,
        BINARY
    };

    GfSTLWriter();

    void Write(const std::string &fileName, 
               const GfTriangleMesh &mesh,
               Format format = ASCII);

  private:
    void _WriteASCII(const std::string &fileName, 
                     const GfTriangleMesh &mesh);
    void _WriteBinary(const std::string &fileName, 
                      const GfTriangleMesh &mesh);
    void _WriteVec3d(std::ofstream &os, const MfVec3d &vec);
    void _WriteVec3dBinary(std::ofstream &os, const MfVec3d &vec);
};

#endif // __GF_STL_WRITER_H__
