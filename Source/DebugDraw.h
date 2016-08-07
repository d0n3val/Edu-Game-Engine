#ifndef __DEBUGDRAW_H__
#define __DEBUGDRAW_H__

#include "Math.h"
#include "Color.h"

void BeginDebugDraw();
void EndDebugDraw();

void DebugDraw(const AABB& aabb, Color color = White);
void DebugDraw(const OBB& obb, Color color = White);
void DebugDraw(const float4x4& transform);
void DebugDraw(const Frustum& frustum, Color color = White);
void DebugDraw(const LineSegment& segment, Color color = White);
void DebugDraw(const float3& point, Color color = White);
void DebugDrawBox(const float3* box_8_vertices, Color color = White);

#endif // __DEBUGDRAW_H__