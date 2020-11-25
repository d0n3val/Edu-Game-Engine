#ifndef __MODULE_EDITOR_CAMERA_H__
#define __MODULE_EDITOR_CAMERA_H__

#include "Module.h"
#include "Globals.h"
#include "Math.h"

class ComponentCamera;
class GameObject;

class ModuleEditorCamera : public Module
{
public:
	ModuleEditorCamera(bool start_enabled = true);
	~ModuleEditorCamera();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void Save(Config* config) const override;
	void Load(Config* config) override;

	void DrawDebug() override;

	void Look(const float3& position);
	void CenterOn(const float3& position, float distance);

	ComponentCamera* GetDummy() const;

private:
	void Move(float dt);
	void Orbit(float motion_x, float motion_y);
	void LookAt(float motion_x, float motion_y);
	void Zoom(float zoom);
	GameObject* Pick(float3* hit_point = nullptr) const;

public: 
	float mov_speed = 1.5f;
	float rot_speed = 1.0f;
	float zoom_speed = 1.5f;

private:

	float3 looking_at;
	bool looking = false;
	ComponentCamera* dummy = nullptr;
	LineSegment picking;
	float3 last_hit;
};

#endif // __MODULE_EDITOR_CAMERA_H__
