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

	void OnDebugDraw() const override;
	void OnUpdateTransform() override;

	float GetNearPlaneDist() const;
	float GetFarPlaneDist() const;
	float GetFOV() const;
	float GetAspectRatio() const;

	void SetNearPlaneDist(float dist);
	void SetFarPlaneDist(float dist);
	void SetFOV(float dist);
	void SetAspectRatio(float dist);


	void Look(const float3& position);

	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

public:
	Frustum frustum;
};

#endif // __COMPONENT_AUDIOCAMERA_H__