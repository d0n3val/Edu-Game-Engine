#ifndef __MODULE_LEVELMANAGER_H__
#define __MODULE_LEVELMANAGER_H__

#include "Globals.h"
#include "Module.h"
#include "Math.h"
#include "QuadTree.h"

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

	void ReceiveEvent(const Event& event) override;
	void DrawDebug() override;

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
	void RecursiveDebugDrawGameObjects(const GameObject* go) const;

	// Add or remove from the hierarchy
	GameObject* CreateGameObject(GameObject * parent, const float3& pos, const float3& scale, const Quat& rot, const char* name = nullptr);
	GameObject* CreateGameObject(GameObject * parent = nullptr);

	// Utils
	GameObject* Validate(const GameObject* pointer) const;
	GameObject* CastRay(const LineSegment& segment, float& dist) const;
	GameObject* CastRay(const Ray& ray, float& dist) const;

private:
	void RecursiveTestRay(const GameObject* go, const LineSegment& segment, float& dist, GameObject** best_candidate) const;
	void RecursiveTestRay(const GameObject* go, const Ray& ray, float& dist, GameObject** best_candidate) const;
	void RecursiveDrawGameObjects(const GameObject* go) const;
	void RecursiveProcessEvent(GameObject* go, const Event& event) const;
	void DestroyFlaggedGameObjects();

public:
	Quadtree quadtree;
	bool draw_quadtree = true;

private:
	GameObject* root = nullptr;
	std::string name;
};

#endif // __MODULE_LEVELMANAGER_H__