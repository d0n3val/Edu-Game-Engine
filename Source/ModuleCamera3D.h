#ifndef __MODULE_CAMERA_3D_H__
#define __MODULE_CAMERA_3D_H__

#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "Math.h"

class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(bool start_enabled = true);
	~ModuleCamera3D();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void Follow(PhysBody3D* body, float min, float max, float height);
	void UnFollow();
	void Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference = false);
	void Move(const vec3 &Movement);
	float* GetViewMatrix();
	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

private:

	void CalculateViewMatrix();

public:
	
	vec3 X, Y, Z, Position, Reference;
	float3 reference;

private:

	mat4x4 ViewMatrix;//, ViewMatrixInverse;
	PhysBody3D* following = nullptr;
	float min_following_dist;
	float max_following_dist;
	float following_height;
	Frustum frustum;
};

#endif // __MODULE_CAMERA_3D_H__