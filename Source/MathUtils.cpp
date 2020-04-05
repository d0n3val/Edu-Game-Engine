#include "Globals.h"

#include "MathUtils.h"

void CatmullRom(const float3& p0, const float3& p1, const float3& p2, const float3& p3, CubicSegment& segment)
{
    float3 m1  = (p2-p0)*0.5f;
    float3 m2  = (p3-p1)*0.5f;

    CatmullRomTangents(p1, p2, m1, m2, segment);
}

void CatmullRomTangents(const float3& p1, const float3& p2, const float3& m1, const float3& m2, CubicSegment& segment)
{
    segment.a = 2.0f*(p1-p2)+m1+m2;
    segment.b = 3.0f*(p2-p1)-2.0f*m1-m2;
    segment.c = m1;
    segment.d = p1;
}

void CentCatmullRom(const float3& p0, const float3& p1, const float3& p2, const float3& p3, CubicSegment& segment, float alpha, float tension)
{
    float t0 = 0.0f;
    float t1 = t0 + pow(p0.Distance(p1), alpha);
    float t2 = t1 + pow(p1.Distance(p2), alpha);
    float t3 = t2 + pow(p2.Distance(p3), alpha);

    float3 m1 = (1.0f - tension) * (t2 - t1) *
        ((p1 - p0) / (t1 - t0) - (p2 - p0) / (t2 - t0) + (p2 - p1) / (t2 - t1));
    float3 m2 = (1.0f - tension) * (t2 - t1) *
        ((p2 - p1) / (t2 - t1) - (p3 - p1) / (t3 - t1) + (p3 - p2) / (t3 - t2));

    segment.a = 2.0f * (p1 - p2) + m1 + m2;
    segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
    segment.c = m1;
    segment.d = p1;}

float3 ApplyCurveSegment(const CubicSegment& segment, float t)
{
    return segment.a*(t*t*t)+segment.b*(t*t)+segment.c*t+segment.d;
}
