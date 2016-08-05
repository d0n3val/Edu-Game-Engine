#ifndef __DEBUGDRAW_H__
#define __DEBUGDRAW_H__

#include "Math.h"
#include "Color.h"

void BeginDebugDraw();
void EndDebugDraw();

void DebugDraw(const AABB& aabb, Color color = White);
void DebugDraw(const OBB& obb, Color color = White);
void DebugDraw(const float4x4& transform);

#endif // __DEBUGDRAW_H__