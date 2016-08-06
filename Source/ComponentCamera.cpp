#include "Globals.h"
#include "ComponentCamera.h"
#include "Application.h"
#include "GameObject.h"

// ---------------------------------------------------------
ComponentCamera::ComponentCamera(GameObject* container) : Component(container)
{
	type = ComponentTypes::Camera;
	frustum.type = FrustumType::PerspectiveFrustum;
}

// ---------------------------------------------------------
ComponentCamera::~ComponentCamera()
{}

// ---------------------------------------------------------
void ComponentCamera::Setup(float near_dist, float far_dist, float fov_degrees, float aspect_ratio)
{
	frustum.nearPlaneDistance = near_dist;
	frustum.farPlaneDistance = far_dist;

	frustum.verticalFov = DEGTORAD * fov_degrees;
	// More about FOV: http://twgljs.org/examples/fov-checker.html
	// fieldOfViewX = 2 * atan(tan(fieldOfViewY * 0.5) * aspect)
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * aspect_ratio);
}

// -----------------------------------------------------------------
void ComponentCamera::UpdateTransformation()
{
	float4x4 trans = game_object->GetGlobalTransformation();

	frustum.pos = trans.TranslatePart();
	frustum.front = trans.WorldZ();
	frustum.up = trans.WorldY();
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
