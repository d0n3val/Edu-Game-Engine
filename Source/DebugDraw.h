#ifndef __DEBUGDRAW_H__
#define __DEBUGDRAW_H__

#include "Math.h"
#include "Color.h"

void BeginDebugDraw();
void EndDebugDraw();

void DebugDraw(const Sphere& sphere, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDraw(const AABB& aabb, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDraw(const OBB& obb, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDraw(const Capsule& capsule, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDraw(const float4x4& transform);
void DebugDraw(const Frustum& frustum, Color color = White);
void DebugDraw(const LineSegment& segment, Color color = White, bool is_arrow = false, const float4x4& transform = float4x4::identity);
void DebugDraw(const Ray& ray, Color color = White, float max_dist = 5000.0f);
void DebugDraw(const float3& point, Color color = White);
void DebugDraw(const Cylinder& cylinder, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDraw(const Cone& cone, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDrawBox(const float3* box_8_vertices, Color color = White);
void DebugDrawArrowZ(const float3& offset = float3::zero, float length = 1.0f, Color color = White, const float4x4& transform = float4x4::identity);
void DebugDrawArrow(const float3 & dir, const float3 & offset, Color color, const float4x4 & transform);
void DebugDrawCircle(const float3& pos, float radius, Color color = White);
void DebugDrawRing(const float3& pos, float radius, float inner_radius, Color color = White);
void DebugDrawArc(const float3& pos, float radius, float angle_left, float angle_right, float inner_radius = 0.0f, Color color = White, const float4x4& transform = float4x4::identity);

#endif // __DEBUGDRAW_H__