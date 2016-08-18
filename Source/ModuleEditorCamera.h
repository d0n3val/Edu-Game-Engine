#ifndef __MODULE_EDITOR_CAMERA_H__
#define __MODULE_EDITOR_CAMERA_H__

#include "Module.h"
#include "Globals.h"
#include "Math.h"

class ComponentCamera;

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

	float3 GetPosition() const;
	void Look(const float3& position);
	void CenterOn(const float3& position, float distance);

	ComponentCamera* GetDummy() const;

private:

	float3 looking_at;
	bool looking = false;
	ComponentCamera* dummy = nullptr;
};

#endif // __MODULE_EDITOR_CAMERA_H__
