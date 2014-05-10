#ifndef __GF_STL_READER_H__
#define __GF_STL_READER_H__

#include <string>

class GfTriangleMesh;
class MfVec3d;

class GfSTLReader
{
  public:
    GfSTLReader();

    GfTriangleMesh * Read(const std::string &fileName);

  private:
    GfTriangleMesh * _ReadASCII(const std::string &fileName);

    GfTriangleMesh * _ReadBinary(const std::string &fileName);

    void _ReportExpectedError(const std::string &expected, 
                              const std::string &found);

    bool _ConsumeToken(std::ifstream &is, const std::string &token);

    MfVec3d _ReadVec3d(std::ifstream &is);

    MfVec3d _ReadVec3dBinary(std::ifstream &is);
};

#endif // __GF_STL_READER_H__
