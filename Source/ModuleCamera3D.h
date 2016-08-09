#ifndef __MODULE_CAMERA_3D_H__
#define __MODULE_CAMERA_3D_H__

#include "Module.h"
#include "Globals.h"
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

	float3 GetPosition() const;
	void Look(const float3& position);
	float* GetViewMatrix();
	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

private:

	Frustum frustum;
};

#endif // __MODULE_CAMERA_3D_H__
