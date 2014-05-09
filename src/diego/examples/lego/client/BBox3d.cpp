#include "BBox3d.h"

#include "TriBox3.h"

#include <iostream>

void
MfBBox3d::ExtendBy(const MfVec3d &point)
{
    if (point[0] < _min[0]) _min[0] = point[0];
    if (point[1] < _min[1]) _min[1] = point[1];
    if (point[2] < _min[2]) _min[2] = point[2];
    if (point[0] > _max[0]) _max[0] = point[0];
    if (point[1] > _max[1]) _max[1] = point[1];
    if (point[2] > _max[2]) _max[2] = point[2];
}

void
MfBBox3d::ExtendBy(const MfBBox3d &bbox)
{
    if (bbox._min[0] < _min[0]) _min[0] = bbox._min[0];
    if (bbox._min[1] < _min[1]) _min[1] = bbox._min[1];
    if (bbox._min[2] < _min[2]) _min[2] = bbox._min[2];
    if (bbox._max[0] > _max[0]) _max[0] = bbox._max[0];
    if (bbox._max[1] > _max[1]) _max[1] = bbox._max[1];
    if (bbox._max[2] > _max[2]) _max[2] = bbox._max[2];
}

bool
MfBBox3d::Intersects(const MfBBox3d &otherBBox) const
{
    // Based on:
    // http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?page=3
    MfVec3d center = GetCenter();
    MfVec3d otherBBoxCenter = otherBBox.GetCenter();
    MfVec3d t = otherBBoxCenter - center;
    MfVec3d halfSize = GetSize() / 2.0f;
    MfVec3d otherBBoxHalfSize = otherBBox.GetSize() / 2.0f;
    return 
        fabs(t[0]) <= (halfSize[0] + otherBBoxHalfSize[0]) &&
        fabs(t[1]) <= (halfSize[1] + otherBBoxHalfSize[1]) &&
        fabs(t[2]) <= (halfSize[2] + otherBBoxHalfSize[2]);
}

bool
MfBBox3d::IntersectsTriangle(const MfVec3d &v0, 
                             const MfVec3d &v1, 
                             const MfVec3d &v2) const
{
    float boxcenter[3];
    float boxhalfsize[3];
    float triverts[3][3];
    for (int i = 0; i < 3; ++i) {
        boxcenter[i] = float((_min[i] + _max[i]) / 2.0);
        boxhalfsize[i] = float((_max[i] - _min[i]) / 2.0);
        triverts[0][i] = float(v0[i]);
        triverts[1][i] = float(v1[i]);
        triverts[2][i] = float(v2[i]);
    }

    return triBoxOverlap(boxcenter, boxhalfsize, triverts) != 0;
}

std::ostream &
operator<<(std::ostream &os, const MfBBox3d &bbox) 
{
    os << "(" << bbox._min << ", " << bbox._max << ")";
    return os;
}
