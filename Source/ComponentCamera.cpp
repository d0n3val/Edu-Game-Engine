#include "Globals.h"
#include "ComponentCamera.h"
#include "Application.h"
#include "GameObject.h"
#include "DebugDraw.h"

// ---------------------------------------------------------
ComponentCamera::ComponentCamera(GameObject* container) : Component(container)
{
	type = ComponentTypes::Camera;
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 5000.0f;
	frustum.verticalFov = DEGTORAD * 60.0f;
	SetAspectRatio(1.3f);
}

// ---------------------------------------------------------
ComponentCamera::~ComponentCamera()
{}

void ComponentCamera::OnDebugDraw() const
{
	DebugDraw(frustum, Yellow);
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
	if(dist > 0.0f && dist < frustum.farPlaneDistance)
		frustum.nearPlaneDistance = dist;
}

// ---------------------------------------------------------
void ComponentCamera::SetFarPlaneDist(float dist)
{
	if(dist > 0.0f && dist > frustum.nearPlaneDistance)
		frustum.farPlaneDistance = dist;
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

