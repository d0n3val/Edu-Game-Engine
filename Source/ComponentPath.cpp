#include "Globals.h"
#include "ComponentPath.h"
#include "Application.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "Imgui/imgui.h"
#include "tinyspline/include/tinysplinecpp.h"
#include <list>

#include "Leaks.h"

using namespace std;
using namespace ts;

// ---------------------------------------------------------
ComponentPath::ComponentPath(GameObject* container) : Component(container, Types::Path)
{
	points.push_back(float3::zero);
	points.push_back(float3::one);
	points.push_back(float3::zero);
	GenerateSpline();
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
		config.AddArrayFloat("Points", &points[0].x, uint(points.size()) * 3);
}

// ---------------------------------------------------------
void ComponentPath::OnLoad(Config * config)
{
	int num_points = config->GetArrayCount("Points");
	if (num_points > 0)
	{
		points.clear();
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
		vector<float3>::const_iterator last = points.end();
		--last;

		for (vector<float3>::const_iterator it = points.begin(); it != last;)
		{
			float3 a = *it;
			float3 b = *(++it);

			dd::line(a, b, dd::colors::Red);
			dd::sphere(a, dd::colors::Yellow, 0.1f);
			if (it == last)
			{
				dd::sphere(b, dd::colors::Yellow, 0.1f);
			}
		}

		if (degrees > 1)
		{
			uint total = (uint) path_lenght;
			for (uint i = 0; i < total;)
			{
				float3 a(spline->evaluate((float)i / (float)total).result_ptr());
				float3 b(spline->evaluate((float)++i / (float)total).result_ptr());
				dd::line(a, b, dd::colors::Green);
			}
		}

		dd::sphere(GetPos(test_point), dd::colors::White, 0.2f);
		dd::sphere(test_close, dd::colors::Yellow, 0.2f);

		test_close_prev = GetClosestPoint(test_close, &test_close_prev);
		dd::sphere(GetClosestPoint(test_close, &test_close_prev), dd::colors::Yellow, 0.2f);
	}
}

// ---------------------------------------------------------
void ComponentPath::DrawEditor()
{
	IMGUI_PRINT("Length: ", "%.3f", path_lenght);

	int num_points = points.size();
	if (ImGui::InputInt("# Points", &num_points))
	{
		if (num_points > 2)
		{
			points.resize(num_points, float3::zero);
			GenerateSpline();
		}
	}

	char name[25];
	for (uint i = 0; i < points.size(); ++i)
	{
		sprintf_s(name, 25, "Point %i", i);
		if (ImGui::DragFloat3(name, &points[i].x))
			GenerateSpline();
	}

	if (ImGui::SliderInt("Degrees", &degrees, 1, points.size()-1))
		GenerateSpline();

	ImGui::SliderFloat("Test Point", &test_point, 0.0f, 1.0f);
	ImGui::DragFloat3("Test Close", &test_close.x);

	test_close_factor = GetClosestPoint(test_close, &test_close_factor);
	IMGUI_PRINT("Close %: ", "%.3f", test_close_factor);
}

// ---------------------------------------------------------
void ComponentPath::GenerateSpline()
{
	if (points.size() > 2)
	{
		RecalculateLength();
		RELEASE(spline);
		spline = new ts::BSpline(degrees, 3, points.size(), TS_CLAMPED);
		spline->setCtrlp(&points[0].x);
	}
}

// ---------------------------------------------------------
float3 ComponentPath::GetPos(float range) const
{
	float3 ret = float3::zero;

	if (spline != nullptr)
	{
		if (range < 0.0f || range > 1.0f)
			range = ModPos(range, 1.0f);
		ret = float3(spline->evaluate(range).result_ptr());
	}

	return ret;
}

// ---------------------------------------------------------
float3 ComponentPath::GetClosestPoint(const float3 & position, const float3 * previous, uint resolution) const
{
	float3 ret = float3::zero;

	if (resolution == 0)
		resolution = (uint)path_lenght;

	std::map<float, LineSegment> closest_segment;

	for (uint i = 0; i < resolution;)
	{
		float3 a(spline->evaluate((float)i / (float)resolution).result_ptr());
		float3 b(spline->evaluate((float)++i / (float)resolution).result_ptr());
		LineSegment ls(a, b);
		float3 closest = ls.ClosestPoint(position);

		float dist = closest.Distance(position);
		if(previous != nullptr)
			dist += closest.Distance(*previous);
		closest_segment[dist] = ls;
	}

	if (closest_segment.size() > 0)
		ret = closest_segment.begin()->second.ClosestPoint(position);

	return ret;
}

// ---------------------------------------------------------
float ComponentPath::GetClosestPoint(const float3 & position, const float * previous, uint resolution) const
{
	float ret = 0.0f;

	if (resolution == 0)
		resolution = (uint)path_lenght;

	std::map<float, float> closest_segment;

	float f1, f2, f3;
	for (uint i = 0; i < resolution; ++i)
	{
		f1 = ((float)i / (float)resolution);
		f2 = ((float)++i / (float)resolution);

		float3 a(spline->evaluate(f1).result_ptr());
		float3 b(spline->evaluate(f2).result_ptr());
		LineSegment ls(a, b);
		float3 closest = ls.ClosestPoint(position, &f3);

		float dist = closest.DistanceSq(position);
		if(previous != nullptr)
			dist += closest.DistanceSq(GetPos(*previous));

		closest_segment[dist] = f1 + ((f2-f1)*f3);
	}

	if (closest_segment.size() > 0)
		ret = closest_segment.begin()->second;

	return ret;
}

// ---------------------------------------------------------
void ComponentPath::RecalculateLength()
{
	path_lenght = 0.0f;

	vector<float3>::const_iterator last = points.end();
	--last;

	for (vector<float3>::const_iterator it = points.begin(); it != last;) 
	{
		float3 a = *it;
		float3 b = *(++it);

		path_lenght += a.Distance(b);
	}
}
