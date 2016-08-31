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

public:
	std::vector<float3> points;
	ts::BSpline* spline = nullptr;

private:
	int degrees = 2;
};

#endif // __COMPONENT_PATH_H__