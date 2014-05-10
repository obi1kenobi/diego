#include "Ray3d.h"

bool
MfRay3d::GetTriangleIntersection(double *depth,
                                 const MfVec3d &v0,
                                 const MfVec3d &v1,
                                 const MfVec3d &v2,
                                 double epsilon) const
{
    /* find vectors for two edges sharing v0 */
    MfVec3d edge1 = v1 - v0;
    MfVec3d edge2 = v2 - v0;

    /* begin calculating determinant - also used to calculate U parameter */
    MfVec3d pvec = _dir.Cross(edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    double det = edge1.Dot(pvec);

    // const double epsilon = 0.00000001f;
    // Known to work with Bunny: const double epsilon = 0.0000000001;
    // Known to fail with Bunny: const double epsilon = 0.0001;
    if (det > -epsilon && det < epsilon)
        return false;
    double inv_det = 1.0f / det;

    /* calculate distance from vert0 to ray origin */
    MfVec3d tvec = _orig - v0;

    /* calculate U parameter and test bounds */
    double u = tvec.Dot(pvec) * inv_det;
    if (u < 0.0 || u > 1.0)
        return false;

    /* prepare to test V parameter */
    MfVec3d qvec = tvec.Cross(edge1);

    /* calculate V parameter and test bounds */
    double v = _dir.Dot(qvec) * inv_det;
    if (v < (0.0 - epsilon) || u + v > (1.0 + epsilon))
        return false;

    /* calculate t, ray intersects triangle */
    if (depth)
        *depth = edge2.Dot(qvec) * inv_det;

    return true;
}
