#ifndef __MF_BBOX3D_H__
#define __MF_BBOX3D_H__

#include "Vec3d.h"

#include <iosfwd>
#include <limits>

class MfBBox3d
{
  public:
    MfBBox3d()
    {
        Clear();
    }

    MfBBox3d(const MfVec3d &min, const MfVec3d &max) :
        _min(min),
        _max(max)
    {
    }

    void Clear() {
        for (int i = 0; i < 3; ++i) {
            _min[i] = std::numeric_limits<double>::max();
            _max[i] = -std::numeric_limits<double>::max();
        }
    }

    void SetMin(const MfVec3d &min) {
        _min = min;
    }
    
    const MfVec3d & GetMin() const {
        return _min;
    }

    void SetMax(const MfVec3d &max) {
        _max = max;
    }

    const MfVec3d & GetMax() const {
        return _max;
    }

    MfVec3d GetCenter() const {
        return (_min + _max) / 2.0f;
    }

    MfVec3d GetSize() const {
        return _max - _min;
    }

    void ExtendBy(const MfVec3d &point);

    void ExtendBy(const MfBBox3d &bbox);

    bool Contains(const MfVec3d &vec) const {
        return 
            vec[0] >= _min[0] && vec[0] <= _max[0] &&
            vec[1] >= _min[1] && vec[1] <= _max[1] &&
            vec[2] >= _min[2] && vec[2] <= _max[2];
    }

    bool Contains(const MfBBox3d &bbox) const {
        return Contains(bbox._min) && Contains(bbox._max);
    }

    bool ContainsTriangle(const MfVec3d &v0, 
                          const MfVec3d &v1, 
                          const MfVec3d &v2) const {
        return Contains(v0) && Contains(v1) && Contains(v2);
    }

    double DistanceToPoint(const MfVec3d &p) const {
        MfVec3d dist;
        for (int i = 0; i < 3; i++) {
            if (p[i] >= _min[i] && p[i] <= _max[i]) {
                dist[i] = 0.0;
            } else {
                double dMin = p[i] - _min[i];
                double dMax = p[i] - _max[i];
                dist[i] = (std::abs(dMin) < std::abs(dMax)) ? dMin : dMax;
            }
        }
        return dist.GetLength();
    }

    bool Intersects(const MfBBox3d &bbox) const;

    bool IntersectsTriangle(const MfVec3d &v0, 
                            const MfVec3d &v1, 
                            const MfVec3d &v2) const;

    friend std::ostream & operator<<(std::ostream &os, const MfBBox3d &bbox);

  private:
    MfVec3d _min;
    MfVec3d _max;
};

#endif // __MF_BBOX3D_H__
