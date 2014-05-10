#ifndef __MF_RAY_3D_H__
#define __MF_RAY_3D_H__

#include "Vec3d.h"

class MfRay3d {
  public:
    MfRay3d(const MfVec3d &orig, const MfVec3d &dir) :
        _orig(orig),
        _dir(dir)
    { 
    }

    const MfVec3d & GetOrigin() const {
        return _orig;
    }

    const MfVec3d & GetDirection() const {
        return _dir;
    }

    bool GetTriangleIntersection(double *depth,
                                 const MfVec3d &v0,
                                 const MfVec3d &v1,
                                 const MfVec3d &v2,
                                 double epsilon = 0.0000000001) const;

  private:
    MfVec3d _orig;
    MfVec3d _dir;
};

#endif // __MF_RAY_3D_H__
