#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#include "Math.h"

struct CubicSegment { float3 a, b, c, d; };

void CatmullRom(const float3& p0, const float3& p1, const float3& p2, const float3& p3, CubicSegment& segment);
void CatmullRomTangents(const float3& p1, const float3& p2, const float3& m1, const float3& m2, CubicSegment& segment);
void CentCatmullRom(const float3& p0, const float3& p1, const float3& p2, const float3& p3, CubicSegment& segment, float alpha, float tension);
float3 ApplyCurveSegment(const CubicSegment& segment, float t);

#endif /* __MATH_UTILS_H__ */
