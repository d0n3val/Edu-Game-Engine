#ifndef __COMPONENT_PATH_H__
#define __COMPONENT_PATH_H__

#include "Globals.h"
#include "Component.h"
#include "Math.h"
#include <list>

namespace ts {
	class BSpline;
}

class ComponentPath : public Component
{
public:
	ComponentPath (GameObject* container);
	~ComponentPath ();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	void OnDebugDraw(bool selected) const override;
	void DrawEditor();

	void GenerateSpline();
	float3 GetPos(float range) const;
	float3 GetClosestPoint(const float3& position, const float3* previous = nullptr, uint resolution = 25) const;

private:
	void RecalculateLength();

public:
	std::vector<float3> points;
	ts::BSpline* spline = nullptr;

private:
	int degrees = 2;
	float path_lenght = 0.0f;
	float test_point = 0.0f;
	float3 test_close = float3::zero;
	mutable float3 test_close_prev = float3::zero;
};

#endif // __COMPONENT_PATH_H__