#ifndef __MF_ROTATION_H__
#define __MF_ROTATION_H__

class MfVec3d;
class MfMatrix4d;

class MfRotation {
  public:
    /// Default constructor leaves rotation undefined.
    MfRotation() {}

    /// Construct a rotation from given quaternion values
    MfRotation(const double q[4]) { 
        SetFromQuaternion(q); 
    }

    /// Construct a rotation from given quaternion values
    MfRotation(double q0, double q1, double q2, double q3) { 
        SetFromQuaternion(q0, q1, q2, q3); 
    }

    /// Construct a rotation from given rotation matrix
    MfRotation(const MfMatrix4d &m) { 
        SetFromRotationMatrix(m); 
    }

    /// Construct a rotation from given 3D axis vector and angle in radians
    MfRotation(const MfVec3d &axis, double radians) { 
        SetFromAxisAngle(axis, radians); 
    }

    /// Construct a rotation from one direction vector to another.
    MfRotation(const MfVec3d &from, const MfVec3d &to) {
        SetFromTwoDirectionVectors(from, to);
    }

    /// Identity (null) rotation
    MfRotation & SetIdentity() { 
        _quat[0] = 0.0;
        _quat[1] = 0.0;
        _quat[2] = 0.0;
        _quat[3] = 1.0;
        return *this;
    }

    /// Set rotation from a quaternion
    MfRotation & SetFromQuaternion(const double q[4]);

    /// Set rotation from a quaternion
    MfRotation & SetFromQuaternion(double q0, double q1, double q2, double q3);

    /// Set rotation from given rotation matrix.
    MfRotation & SetFromRotationMatrix(const MfMatrix4d &m);

    /// Set rotation from given 3D axis vector and angle in radians.
    MfRotation & SetFromAxisAngle(const MfVec3d &axis, double radians);

    /// Set rotation from one direction vector to another.
    MfRotation & SetFromTwoDirectionVectors(const MfVec3d &rotateFrom, 
                                            const MfVec3d &rotateTo);

    /// Extracts rotation as quaternion values.
    void GetQuaternion(double *q0, double *q1, double *q2, double *q3) const {
        *q0 = _quat[0];
        *q1 = _quat[1];
        *q2 = _quat[2];
        *q3 = _quat[3];
    }

    /// Extracts rotation as 3D axis vector and angle in radians
    void GetAxisAngle(MfVec3d *axis, double *radians) const;

    /// Extracts rotation as a rotation matrix.
    void GetRotationMatrix(MfMatrix4d *matrix) const;

    /// Changes a rotation to be its inverse
    MfRotation & Invert();

    /// Get inverse rotation of this rotation
    MfRotation GetInverse() const { 
        MfRotation rot = *this; 
        return rot.Invert(); 
    }

    /// Rotate given vector.
    MfVec3d Rotate(const MfVec3d &vec) const;

    /// Rotation multiplication.
    friend MfRotation operator*(const MfRotation &rot1, const MfRotation &rot2);

    /// Rotation multiplication.
    MfRotation & operator*=(const MfRotation &rot);

    /// Equality operator
    friend bool	operator==(const MfRotation &rot1, const MfRotation &rot2) {
        return (rot1._quat[0] == rot2._quat[0] &&
                rot1._quat[1] == rot2._quat[1] &&
                rot1._quat[2] == rot2._quat[2] &&
                rot1._quat[3] == rot2._quat[3]);
    }

    // Inequality operator
    friend bool	operator!=(const MfRotation &rot1, const MfRotation &rot2) { 
        return !(rot1 == rot2); 
    }

  private:
    void _Normalize();
    double _GetNorm() const;

    double	_quat[4];
};

#endif // __MF_ROTATION_H__
