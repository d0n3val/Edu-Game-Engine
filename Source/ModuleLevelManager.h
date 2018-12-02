#ifndef __MODULE_LEVELMANAGER_H__
#define __MODULE_LEVELMANAGER_H__

#include "Globals.h"
#include "Module.h"
#include "Math.h"
#include "QuadTree.h"
#include "DirLight.h"
#include "AmbientLight.h"

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


    const DirLight&     GetDirLight             () const { return directional; }
    DirLight&           GetDirLight             () { return directional; }

    const AmbientLight& GetAmbientLight         () const { return ambient; }
    AmbientLight&       GetAmbientLight         () { return ambient; }


private:

	void RecursiveTestRayBBox(const LineSegment& segment, float& dist, float3& normal, GameObject** best_candidate) const;
	void RecursiveTestRay(const LineSegment& segment, float& dist, GameObject** best_candidate) const;
	void RecursiveTestRay(const Ray& ray, float& dist, GameObject** best_candidate) const;
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
    AmbientLight ambient;
    DirLight directional;
	std::string name;
};

#endif // __MODULE_LEVELMANAGER_H__
