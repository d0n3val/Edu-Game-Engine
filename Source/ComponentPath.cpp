#include "Globals.h"
#include "ComponentPath.h"
#include "Application.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "Imgui/imgui.h"
#include "tinyspline/include/tinysplinecpp.h"
#include <list>

using namespace std;
using namespace ts;

// ---------------------------------------------------------
ComponentPath::ComponentPath(GameObject* container) : Component(container, Types::Path)
{
}

// ---------------------------------------------------------
ComponentPath::~ComponentPath()
{
	RELEASE(spline);
}

// ---------------------------------------------------------
void ComponentPath::OnSave(Config& config) const
{
	if(points.size() > 0)
		config.AddArrayFloat("Points", &points[0].x, points.size() * 3);
}

// ---------------------------------------------------------
void ComponentPath::OnLoad(Config * config)
{
	int num_points = config->GetArrayCount("Points");
	if (num_points > 0)
	{
		points.reserve(num_points / 3);
		for (int i = 0; i < num_points;)
		{
			points.push_back( float3(
					config->GetFloat("Points", 0.0f, i++),
					config->GetFloat("Points", 0.0f, i++),
					config->GetFloat("Points", 0.0f, i++) ));
		}
		GenerateSpline();
	}
}

// ---------------------------------------------------------
void ComponentPath::OnDebugDraw(bool selected) const
{
	if (selected == true)
	{
		if (points.size() >= 2)
		{
			vector<float3>::const_iterator last = points.end();
			--last;

			for (vector<float3>::const_iterator it = points.begin(); it != last;)
			{
				float3 a = *it;
				float3 b = *(++it);

				DebugDraw(LineSegment(a,b), Red);
				DebugDraw(Sphere(a, 0.1f), Yellow);
			}
		}

		if (spline != nullptr)
		{
			uint total = 25;
			for (uint i = 0; i < total - 1;)
			{
				float3 a(spline->evaluate((float)i / (float)total).result_ptr());
				float3 b(spline->evaluate((float)++i / (float)total).result_ptr());
				DebugDraw(LineSegment(a, b), Green);
			}
		}
	}
}

// ---------------------------------------------------------
void ComponentPath::DrawEditor()
{
	int num_points = points.size();
	if (ImGui::InputInt("# Points", &num_points))
		points.resize(num_points, float3::zero);

	char name[25];
	for (uint i = 0; i < points.size(); ++i)
	{
		sprintf_s(name, 25, "Point %i", i);
		ImGui::DragFloat3(name, &points[i].x);
	}

	ImGui::SliderInt("Spline Degrees", &degrees, 1, points.size() - 1);

	if (ImGui::Button("Generate Spline"))
		GenerateSpline();
}

// ---------------------------------------------------------
void ComponentPath::GenerateSpline()
{
	if (points.size() > 2)
	{
		RELEASE(spline);
		spline = new ts::BSpline(degrees, 3, points.size(), TS_CLAMPED);
		spline->setCtrlp(&points[0].x);
	}
}

// ---------------------------------------------------------
float3 ComponentPath::GetPos(float range) const
{
	return float3(spline->evaluate(range).result_ptr());
}
