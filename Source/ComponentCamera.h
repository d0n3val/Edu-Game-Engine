#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Globals.h"
#include "Component.h"
#include "Math.h"

class ComponentCamera : public Component
{
	friend class ModuleAudio;
public:
	ComponentCamera (GameObject* container);
	~ComponentCamera ();

	void Setup(float near_dist, float far_dist, float fov_degrees, float aspect_ratio);
	void UpdateTransformation();

	void Look(const float3& position);

	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

private:
	Frustum frustum;
};

#endif // __COMPONENT_AUDIOCAMERA_H__