#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#include "Math.h"

template<class T>
struct CubicSegment { T a, b, c, d; };

using CubicSegment3 = CubicSegment<float3>;
using CubicSegment2 = CubicSegment<float2>;

template<class T>
inline void CatmullRom(const T& p0, const T& p1, const T& p2, const T& p3, CubicSegment<T>& segment)
{
    T m1  = (p2-p0)*0.5f;
    T m2  = (p3-p1)*0.5f;

    CatmullRomTangents(p1, p2, m1, m2, segment);
}

template<class T>
inline void CatmullRomTangents(const T& p1, const T& p2, const T& m1, const T& m2, CubicSegment<T>& segment)
{
    segment.a = 2.0f*(p1-p2)+m1+m2;
    segment.b = 3.0f*(p2-p1)-2.0f*m1-m2;
    segment.c = m1;
    segment.d = p1;
}

template<class T>
inline void CentCatmullRom(const T& p0, const T& p1, const T& p2, const T& p3, CubicSegment<T>& segment, float alpha, float tension)
{
    float t0 = 0.0f;
    float t1 = t0 + pow(p0.Distance(p1), alpha);
    float t2 = t1 + pow(p1.Distance(p2), alpha);
    float t3 = t2 + pow(p2.Distance(p3), alpha);

    T m1 = T::zero;
    T m2 = T::zero;

    if (t0+0.00001f < t1)
    {
        m1 = (1.0f - tension) * (t2 - t1) *
            ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1));
    }

    if (t2 + 0.00001f < t3)
    {
        m2 = (1.0f - tension) * (t2 - t1) *
            ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2));
    }

    segment.a = 2.0f * (p1 - p2) + m1 + m2;
    segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
    segment.c = m1;
    segment.d = p1;
}

template<class T>
inline T ApplyCurveSegment(const CubicSegment<T>& segment, float t)
{
    return segment.a*(t*t*t)+segment.b*(t*t)+segment.c*t+segment.d;
}

#endif /* __MATH_UTILS_H__ */
