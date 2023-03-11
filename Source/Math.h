#ifndef __MATH_H__
#define __MATH_H__

#include "MathGeoLib/include/MathBuildConfig.h"
#include "MathGeoLib/include/MathGeoLib.h"

inline float3 QuatToEuler(const Quat& q)
{
    float3 res; 

    // roll 
    res.z = atan2(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.z*q.z+q.x*q.x) );

    // pitch
    res.y = 2.0f*atan2(sqrt(1.0f + 2.0f * (q.w * q.x - q.z * q.y)), sqrt(1.0f - 2.0f * (q.w * q.x - q.z * q.y))) - math::pi/2;

    // yaw
    res.x = atan2(2.0f * (q.w * q.y + q.z * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));

    return res;
}

inline Quat QuatFromEuler(const float3& euler)
{
    float cr = cos(euler.z * 0.5f);
    float sr = sin(euler.z * 0.5f);
    float cp = cos(euler.y * 0.5f);
    float sp = sin(euler.y * 0.5f);
    float cy = cos(euler.x * 0.5f);
    float sy = sin(euler.x * 0.5f);

    Quat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = cr * sp * cy + sr * cp * sy;
    q.y = cr * cp * sy - sr * sp * cy;
    q.z = sr * cp * cy - cr * sp * sy;

    return q;
}


#endif // __MATH_H__