#include "Globals.h"
#include "ComponentCamera.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleLevelManager.h"
#include "DebugDraw.h"

#include "Leaks.h"

// ---------------------------------------------------------
ComponentCamera::ComponentCamera(GameObject* container) : Component(container, Types::Camera)
{
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos = float3::zero;
	frustum.front = float3::unitZ;
	frustum.up = float3::unitY;

	frustum.nearPlaneDistance = 100.0f;
	frustum.farPlaneDistance = 10000.0f;
	frustum.verticalFov = DEGTORAD * 60.0f;
	SetAspectRatio(1.3f);
}

// ---------------------------------------------------------
ComponentCamera::~ComponentCamera()
{}

// ---------------------------------------------------------
void ComponentCamera::OnStart()
{
}

// ---------------------------------------------------------
void ComponentCamera::OnUpdate(float dt)
{
}

// ---------------------------------------------------------
void ComponentCamera::OnDebugDraw(bool selected) const
{
	if (selected == true)
	{
		float4x4 matrix = GetOpenGLProjectionMatrix()*GetOpenGLViewMatrix();
		matrix.Inverse();
		dd::frustum(matrix, dd::colors::Yellow);
	}
}

// ---------------------------------------------------------
void ComponentCamera::OnSave(Config& config) const
{
	config.AddArrayFloat("Frustum", (float*)&frustum.pos.x, 13);
}

// ---------------------------------------------------------
void ComponentCamera::OnLoad(Config * config)
{
	uint looking_at_uid = config->GetUInt("Looking At", 0);

	frustum.pos.x = config->GetFloat("Frustum", 0.f, 0);
	frustum.pos.y = config->GetFloat("Frustum", 0.f, 1);
	frustum.pos.z = config->GetFloat("Frustum", 0.f, 2);

	frustum.front.x = config->GetFloat("Frustum", 0.f, 3);
	frustum.front.y = config->GetFloat("Frustum", 0.f, 4);
	frustum.front.z = config->GetFloat("Frustum", 1.f, 5);

	frustum.up.x = config->GetFloat("Frustum", 0.f, 6);
	frustum.up.y = config->GetFloat("Frustum", 1.f, 7);
	frustum.up.z = config->GetFloat("Frustum", 0.f, 8);

	frustum.nearPlaneDistance = config->GetFloat("Frustum", 100.0f, 9);
	frustum.farPlaneDistance = config->GetFloat("Frustum", 10000.f, 10);

	frustum.horizontalFov = config->GetFloat("Frustum", 1.f, 11);
	frustum.verticalFov = config->GetFloat("Frustum", 1.f, 12);
}

// -----------------------------------------------------------------
void ComponentCamera::OnUpdateTransform()
{
	float4x4 trans = game_object->GetGlobalTransformation();

	frustum.pos = trans.TranslatePart();
	frustum.front = -trans.WorldZ();
	frustum.up = trans.WorldY();
}

void ComponentCamera::OnUpdateFrustum()
{
	if(game_object)
	{
		game_object->SetLocalTransform(GetCameraMatrix());
	}
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
	}
}

// ---------------------------------------------------------
void ComponentCamera::SetFarPlaneDist(float dist)
{
	if (dist > 0.0f && dist > frustum.nearPlaneDistance)
	{
		frustum.farPlaneDistance = dist;
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
float4x4 ComponentCamera::GetOpenGLViewMatrix() const
{
	float4x4 m;
	
	m = frustum.ViewMatrix();
	m.Transpose();

	return m;
}

// -----------------------------------------------------------------
float4x4 ComponentCamera::GetOpenGLProjectionMatrix() const
{
	float4x4 m;
	
	m = frustum.ProjectionMatrix();
	m.Transpose();

	return m;
}

// -----------------------------------------------------------------
float4x4 ComponentCamera::GetCameraMatrix() const
{
    return frustum.WorldMatrix();
}

// -----------------------------------------------------------------
float4x4 ComponentCamera::GetViewMatrix() const
{
    return frustum.ViewMatrix();
}

// -----------------------------------------------------------------
float4x4 ComponentCamera::GetProjectionMatrix() const
{
    return frustum.ProjectionMatrix();
	//return float4x4::D3DPerspProjRH(frustum.nearPlaneDistance, frustum.farPlaneDistance, frustum.NearPlaneWidth(), frustum.NearPlaneHeight());
}

void ComponentCamera::GetPlanes(float4* planes) const
{
    Plane tmp[6];
    frustum.GetPlanes(tmp);
    for (uint i = 0; i < 6; ++i) planes[i] = float4(tmp[i].normal, tmp[i].d);

}

