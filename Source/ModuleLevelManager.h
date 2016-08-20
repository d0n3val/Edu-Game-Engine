#ifndef __MODULE_LEVELMANAGER_H__
#define __MODULE_LEVELMANAGER_H__

#include "Globals.h"
#include "Module.h"
#include "Math.h"

class GameObject;

class ModuleLevelManager : public Module
{
public:

	ModuleLevelManager(bool start_enabled = true);
	~ModuleLevelManager();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	bool CleanUp() override;

	// Utils
	const GameObject* GetRoot() const;
	GameObject* GetRoot();

	// Manage whole levels
	bool CreateNewEmpty(const char* name);
	bool Load(const char* file);
	bool Save(const char* file = nullptr);
	void UnloadCurrent();
	GameObject* Find(uint serialization_id, const GameObject* from) const;
	
	// Draw
	void Draw() const;

	// Add or remove from the hierarchy
	GameObject* CreateGameObject(GameObject * parent, const float3& pos, const float3& scale, const Quat& rot, const char* name = nullptr);
	GameObject* CreateGameObject(GameObject * parent = nullptr);

	void RecursiveDebugDrawGameObjects(const GameObject* go) const;

	GameObject* Validate(const GameObject* pointer) const;

private:
	void RecursiveDrawGameObjects(const GameObject* go) const;
	void DestroyFlaggedGameObjects();

private:
	GameObject* root = nullptr;
	std::string name;
};

#endif // __MODULE_LEVELMANAGER_H__