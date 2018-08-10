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
	update_status Update(float dt) override;
	bool CleanUp() override;

	void ReceiveEvent(const Event& event) override;
	void DrawDebug() override;

	// Utils
	const GameObject*   GetRoot() const;
	GameObject*         GetRoot();
	const GameObject*   Find(uint uid) const;
	GameObject*         Find(uint uid);

	// Manage whole levels
	bool CreateNewEmpty(const char* name);
	bool Load(const char* file);
	bool Save(const char* file = nullptr);
	void UnloadCurrent();
	
	// Draw
	void Draw() const;
	void RecursiveDebugDrawGameObjects(const GameObject* go) const;

	// Add or remove from the hierarchy
	GameObject* CreateGameObject(GameObject * parent, const float3& pos, const float3& scale, const Quat& rot, const char* name = nullptr);
	GameObject* CreateGameObject(GameObject * parent = nullptr);
	GameObject* Duplicate(const GameObject* original);

	// Utils
	GameObject*         Validate                (const GameObject* pointer) const;
	GameObject*         CastRay                 (const LineSegment& segment, float& dist) const;
	GameObject*         CastRay                 (const Ray& ray, float& dist) const;
	GameObject*         CastRayOnBoundingBoxes  (const LineSegment& segment, float& dist, float3& normal) const;
	void                FindNear                (const float3& position, float radius, std::vector<GameObject*>& results) const;

	GameObject*  	    AddPointLight 	        (const float3& position);
	GameObject*  	    AddDirectionalLight 	(const float3& direction, const float3& up);
    GameObject*         GetActiveLight          ();
    const GameObject*   GetActiveLight          () const ;

private:

	void RecursiveTestRayBBox(const LineSegment& segment, float& dist, float3& normal, GameObject** best_candidate) const;
	void RecursiveTestRay(const LineSegment& segment, float& dist, GameObject** best_candidate) const;
	void RecursiveTestRay(const Ray& ray, float& dist, GameObject** best_candidate) const;
	void RecursiveDrawGameObjects(const GameObject* go) const;
	void RecursiveProcessEvent(GameObject* go, const Event& event) const;
	void RecursiveUpdate(GameObject* go, float dt) const;
	GameObject* RecursiveFind(uint uid, GameObject* go) const;
	void DestroyFlaggedGameObjects();

	void LoadGameObjects(const Config& config);

public:
	Quadtree quadtree;
	bool draw_quadtree = false;

private:
	GameObject* root = nullptr;
	GameObject* active_light = nullptr;
	std::string name;
};

inline GameObject* ModuleLevelManager::GetActiveLight()
{
    return active_light;
}

inline const GameObject* ModuleLevelManager::GetActiveLight() const 
{
    return active_light;
}

#endif // __MODULE_LEVELMANAGER_H__
