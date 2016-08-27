#include "Globals.h"
#include "ComponentCamera.h"
#include "Application.h"
#include "GameObject.h"
#include "DebugDraw.h"

// ---------------------------------------------------------
ComponentCamera::ComponentCamera(GameObject* container) : Component(container, Types::Camera)
{
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos = float3::zero;
	frustum.front = float3::unitZ;
	frustum.up = float3::unitY;

	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance = 1000.0f;
	frustum.verticalFov = DEGTORAD * 60.0f;
	SetAspectRatio(1.3f);

	background = Black;
	projection_changed = true;
}

// ---------------------------------------------------------
ComponentCamera::~ComponentCamera()
{}

// ---------------------------------------------------------
void ComponentCamera::OnDebugDraw(bool selected) const
{
	if(selected == true)
		DebugDraw(frustum, Yellow);
}

// ---------------------------------------------------------
void ComponentCamera::OnSave(Config& config) const
{
	config.AddArrayFloat("Background", (float*) &background, 4);
	config.AddArrayFloat("Frustum", (float*)&frustum.pos.x, 13);
}

// ---------------------------------------------------------
void ComponentCamera::OnLoad(Config * config)
{
	background.r = config->GetFloat("Background", 0.f, 0);
	background.g = config->GetFloat("Background", 0.f, 1);
	background.b = config->GetFloat("Background", 0.f, 2);
	background.a = config->GetFloat("Background", 1.f, 3);

	frustum.pos.x = config->GetFloat("Frustum", 0.f, 0);
	frustum.pos.y = config->GetFloat("Frustum", 0.f, 1);
	frustum.pos.z = config->GetFloat("Frustum", 0.f, 2);

	frustum.front.x = config->GetFloat("Frustum", 0.f, 3);
	frustum.front.y = config->GetFloat("Frustum", 0.f, 4);
	frustum.front.z = config->GetFloat("Frustum", 1.f, 5);

	frustum.up.x = config->GetFloat("Frustum", 0.f, 6);
	frustum.up.y = config->GetFloat("Frustum", 1.f, 7);
	frustum.up.z = config->GetFloat("Frustum", 0.f, 8);

	frustum.nearPlaneDistance = config->GetFloat("Frustum", 0.1f, 9);
	frustum.farPlaneDistance = config->GetFloat("Frustum", 1000.f, 10);

	frustum.horizontalFov = config->GetFloat("Frustum", 1.f, 11);
	frustum.verticalFov = config->GetFloat("Frustum", 1.f, 12);

	projection_changed = true;
}

// -----------------------------------------------------------------
void ComponentCamera::OnUpdateTransform()
{
	float4x4 trans = game_object->GetGlobalTransformation();

	frustum.pos = trans.TranslatePart();
	frustum.front = trans.WorldZ();
	frustum.up = trans.WorldY();
}

// ---------------------------------------------------------
float ComponentCamera::GetNearPlaneDist() const
{
	return frustum.nearPlaneDistance;
}

// ---------------------------------------------------------
float ComponentCamera::GetFarPlaneDist() const
{
	return frustum.farPlaneDistance;
}

// ---------------------------------------------------------
float ComponentCamera::GetFOV() const
{
	return frustum.verticalFov * RADTODEG;
}

// ---------------------------------------------------------
float ComponentCamera::GetAspectRatio() const
{
	return frustum.AspectRatio();
}

// ---------------------------------------------------------
void ComponentCamera::SetNearPlaneDist(float dist)
{
	if (dist > 0.0f && dist < frustum.farPlaneDistance)
	{
		frustum.nearPlaneDistance = dist;
		projection_changed = true;
	}
}

// ---------------------------------------------------------
void ComponentCamera::SetFarPlaneDist(float dist)
{
	if (dist > 0.0f && dist > frustum.nearPlaneDistance)
	{
		frustum.farPlaneDistance = dist;
		projection_changed = true;
	}
}

// ---------------------------------------------------------
void ComponentCamera::SetFOV(float fov)
{
	float aspect_ratio = frustum.AspectRatio();

	frustum.verticalFov = DEGTORAD * fov;
	SetAspectRatio(aspect_ratio);
}

// ---------------------------------------------------------
void ComponentCamera::SetAspectRatio(float aspect_ratio)
{
	// More about FOV: http://twgljs.org/examples/fov-checker.html
	// fieldOfViewX = 2 * atan(tan(fieldOfViewY * 0.5) * aspect)
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * aspect_ratio);
	projection_changed = true;
}

// -----------------------------------------------------------------
void ComponentCamera::Look(const float3& position)
{
	float3 dir = position - frustum.pos;

	float3x3 m = float3x3::LookAt(frustum.front, dir.Normalized(), frustum.up, float3::unitY);

	frustum.front = m.MulDir(frustum.front).Normalized();
	frustum.up = m.MulDir(frustum.up).Normalized();
}

// -----------------------------------------------------------------
float * ComponentCamera::GetOpenGLViewMatrix()
{
	static float4x4 m;
	
	m = frustum.ViewMatrix();
	m.Transpose();

	return (float*) m.v;
}

// -----------------------------------------------------------------
float * ComponentCamera::GetOpenGLProjectionMatrix()
{
	static float4x4 m;
	
	m = frustum.ProjectionMatrix();
	m.Transpose();

	return (float*) m.v;
}

