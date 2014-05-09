#include "Rotation.h"

#include "Vec3d.h"
#include "Matrix4d.h"

#include <cmath>

MfRotation &
MfRotation::SetFromQuaternion(const double q[4])
{
    _quat[0] = q[0];
    _quat[1] = q[1];
    _quat[2] = q[2];
    _quat[3] = q[3];
    _Normalize();
    return *this;
}

MfRotation &
MfRotation::SetFromQuaternion(double q0, double q1, double q2, double q3)
{
    _quat[0] = q0;
    _quat[1] = q1;
    _quat[2] = q2;
    _quat[3] = q3;
    _Normalize();
    return *this;
}

// Based on "Quaternions and 4x4 Matrices" by Ken Shoemake, Graphics Gems II.
MfRotation &
MfRotation::SetFromRotationMatrix(const MfMatrix4d &m)
{
    int i, j, k;

    if (m[0][0] > m[1][1]) {
	if (m[0][0] > m[2][2]) {
	    i = 0;
	} else {
            i = 2; 
        }
    } else {
	if (m[1][1] > m[2][2]) {
	    i = 1;
	} else {
            i = 2;
        }
    }
    if (m[0][0] + m[1][1] + m[2][2] > m[i][i]) {
	_quat[3] = sqrt(m[0][0] + m[1][1] + m[2][2] + m[3][3]) / 2.0;
	_quat[0] = (m[1][2] - m[2][1]) / (4 * _quat[3]);
	_quat[1] = (m[2][0] - m[0][2]) / (4 * _quat[3]);
	_quat[2] = (m[0][1] - m[1][0]) / (4 * _quat[3]);
    } else {
	j = (i + 1) % 3; 
        k = (i + 2) % 3;
	_quat[i] = sqrt(m[i][i] - m[j][j] - m[k][k] + m[3][3]) / 2.0;
	_quat[j] = (m[i][j] + m[j][i]) / (4 * _quat[i]);
	_quat[k] = (m[i][k] + m[k][i]) / (4 * _quat[i]);
	_quat[3] = (m[j][k] - m[k][j]) / (4 * _quat[i]);
    }

    return *this;
}

MfRotation &
MfRotation::SetFromAxisAngle(const MfVec3d &axis, double radians)
{
    MfVec3d qvec = axis;
    qvec.Normalize();
    qvec *= std::sin(radians / 2.0);
    _quat[0] = qvec[0];
    _quat[1] = qvec[1];
    _quat[2] = qvec[2];
    _quat[3] = std::cos(radians / 2.0);
    return *this;
}

MfRotation &
MfRotation::SetFromTwoDirectionVectors(const MfVec3d &rotateFrom, 
                                       const MfVec3d &rotateTo)
{
    MfVec3d from = rotateFrom;
    MfVec3d to = rotateTo;
    MfVec3d axis;
    double cost;

    from.Normalize();
    to.Normalize();
    cost = from.Dot(to);

    // check for degeneracies
    if (cost > 0.99999) {
        // vectors are parallel
	_quat[0] = _quat[1] = _quat[2] = 0.0; _quat[3] = 1.0;
	return *this;
    } else if (cost < -0.99999) {
        // vectors are opposite

	// find an axis to rotate around, which should be
	// perpendicular to the original axis
	// Try cross product with (1,0,0) first, if that's one of our
	// original vectors then try  (0,1,0).
	MfVec3d tmp = from.Cross(MfVec3d(1.0, 0.0, 0.0));
	if (tmp.GetLength() < 0.00001) {
	    tmp = from.Cross(MfVec3d(0.0, 1.0, 0.0));
        }

	tmp.Normalize();
	SetFromQuaternion(tmp[0], tmp[1], tmp[2], 0.0);
	return *this;
    }

    axis = rotateFrom.Cross(rotateTo);
    axis.Normalize();

    // use half-angle formulae
    // sin^2 t = ( 1 - cos (2t) ) / 2
    axis *= sqrt(0.5 * (1.0 - cost));

    // scale the axis by the sine of half the rotation angle to get
    // the normalized quaternion
    _quat[0] = axis[0];
    _quat[1] = axis[1];
    _quat[2] = axis[2];

    // cos^2 t = ( 1 + cos (2t) ) / 2
    // w part is cosine of half the rotation angle
    _quat[3] = sqrt(0.5 * (1.0 + cost));

    return *this;
}

void
MfRotation::GetAxisAngle(MfVec3d *axis, double *radians) const
{
    MfVec3d qvec(_quat[0], _quat[1], _quat[2]);
    double length = qvec.GetLength();
    const double epsilon = 0.0001;
    if (length > epsilon) {
	*axis = qvec * (1.0 / length);
	*radians = 2.0 * std::acos(_quat[3]);
    } else {
        // Null rotation
	axis->Set(0.0, 0.0, 1.0);
	*radians = 0.0;
    }
}

void
MfRotation::GetRotationMatrix(MfMatrix4d *matrix) const
{
    MfMatrix4d &mat = *matrix;

    mat[0][0] = 1 - 2.0 * (_quat[1] * _quat[1] + _quat[2] * _quat[2]);
    mat[0][1] =     2.0 * (_quat[0] * _quat[1] + _quat[2] * _quat[3]);
    mat[0][2] =     2.0 * (_quat[2] * _quat[0] - _quat[1] * _quat[3]);
    mat[0][3] = 0.0;

    mat[1][0] =     2.0 * (_quat[0] * _quat[1] - _quat[2] * _quat[3]);
    mat[1][1] = 1 - 2.0 * (_quat[2] * _quat[2] + _quat[0] * _quat[0]);
    mat[1][2] =     2.0 * (_quat[1] * _quat[2] + _quat[0] * _quat[3]);
    mat[1][3] = 0.0;

    mat[2][0] =     2.0 * (_quat[2] * _quat[0] + _quat[1] * _quat[3]);
    mat[2][1] =     2.0 * (_quat[1] * _quat[2] - _quat[0] * _quat[3]);
    mat[2][2] = 1 - 2.0 * (_quat[1] * _quat[1] + _quat[0] * _quat[0]);
    mat[2][3] = 0.0;

    mat[3][0] = 0.0;
    mat[3][1] = 0.0;
    mat[3][2] = 0.0;
    mat[3][3] = 1.0;
}

MfRotation &
MfRotation::Invert()
{
    double inverseNorm = 1.0 / _GetNorm();
    _quat[0] = -_quat[0] * inverseNorm;
    _quat[1] = -_quat[1] * inverseNorm;
    _quat[2] = -_quat[2] * inverseNorm;
    _quat[3] =  _quat[3] * inverseNorm;
    return *this;
}

MfVec3d
MfRotation::Rotate(const MfVec3d &vec) const
{
    MfMatrix4d mat;
    mat.SetRotation(*this);
    return mat.Transform(vec);
}

MfRotation &
MfRotation::operator*=(const MfRotation &rot)
{
    _quat[0] =
        rot._quat[3] * _quat[0] + rot._quat[0] * _quat[3] + 
        rot._quat[1] * _quat[2] - rot._quat[2] * _quat[1];
    _quat[1] =
        rot._quat[3] * _quat[1] + rot._quat[1] * _quat[3] + 
        rot._quat[2] * _quat[0] - rot._quat[0] * _quat[2];
    _quat[2] =
        rot._quat[3] * _quat[2] + rot._quat[2] * _quat[3] +
        rot._quat[0] * _quat[1] - rot._quat[1] * _quat[0];
    _quat[3] =
        rot._quat[3] * _quat[3] - rot._quat[0] * _quat[0] -
        rot._quat[1] * _quat[1] - rot._quat[2] * _quat[2];

    _Normalize();

    return *this;
}

MfRotation
operator*(const MfRotation &rot1, const MfRotation &rot2)
{
    MfRotation rot(rot2._quat[3] * rot1._quat[0] + rot2._quat[0] * rot1._quat[3] +
                   rot2._quat[1] * rot1._quat[2] - rot2._quat[2] * rot1._quat[1],
                   rot2._quat[3] * rot1._quat[1] + rot2._quat[1] * rot1._quat[3] +
                   rot2._quat[2] * rot1._quat[0] - rot2._quat[0] * rot1._quat[2],
                   rot2._quat[3] * rot1._quat[2] + rot2._quat[2] * rot1._quat[3] +
                   rot2._quat[0] * rot1._quat[1] - rot2._quat[1] * rot1._quat[0],
                   rot2._quat[3] * rot1._quat[3] - rot2._quat[0] * rot1._quat[0] -
                   rot2._quat[1] * rot1._quat[1] - rot2._quat[2] * rot1._quat[2]);
    rot._Normalize();
    return rot;
}

double
MfRotation::_GetNorm() const
{
    return 
        _quat[0] * _quat[0] + 
        _quat[1] * _quat[1] + 
        _quat[2] * _quat[2] + 
        _quat[3] * _quat[3];
}

void
MfRotation::_Normalize()
{
    double invMagnitude = 1.0 / sqrt(_GetNorm());
    _quat[0] *= invMagnitude;
    _quat[1] *= invMagnitude;
    _quat[2] *= invMagnitude;
    _quat[3] *= invMagnitude;
}
