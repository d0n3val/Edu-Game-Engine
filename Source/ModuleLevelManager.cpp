#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleFileSystem.h"
#include "ModuleSceneLoader.h"
#include "GameObject.h"
#include "Config.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ModuleEditor.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
#include "Event.h"

using namespace std;

ModuleLevelManager::ModuleLevelManager( bool start_enabled) : Module("LevelManager", start_enabled)
{}

// Destructor
ModuleLevelManager::~ModuleLevelManager()
{}

// Called before render is available
bool ModuleLevelManager::Init(Config* config)
{
	bool ret = true;
	LOG("Loading Level Manager");
	
	// create an empty game object to be the root of everything
	root = new GameObject(nullptr, "root");
	quadtree.SetBoundaries(AABB(float3(-500,0,-500), float3(500,30,500)));

	return ret;
}

bool ModuleLevelManager::Start(Config * config)
{
	// Load a default map
	Load("default.eduscene");

	return true;
}

update_status ModuleLevelManager::PreUpdate(float dt)
{
	DestroyFlaggedGameObjects();
	// Update transformations tree for this frame
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), false);
	root->RecursiveCalcBoundingBoxes();

	return UPDATE_CONTINUE;
}

update_status ModuleLevelManager::Update(float dt)
{
	if(App->IsPlay())
		RecursiveUpdate(root, dt);

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleLevelManager::CleanUp()
{
	LOG("Freeing Level Manager");

	// This recursively must destroy all gameobjects
	RELEASE(root);

	return true;
}

void ModuleLevelManager::ReceiveEvent(const Event & event)
{
	RecursiveProcessEvent(root, event);
}

const GameObject * ModuleLevelManager::GetRoot() const
{
	return root;
}

GameObject * ModuleLevelManager::GetRoot()
{
	return root;
}

const GameObject * ModuleLevelManager::Find(uint uid) const
{
	if (uid > 0)
		return RecursiveFind(uid, root);

	return nullptr;
}

GameObject * ModuleLevelManager::Find(uint uid)
{
	if (uid > 0)
		return RecursiveFind(uid, root);

	return nullptr;
}

bool ModuleLevelManager::CreateNewEmpty(const char * name)
{
	UnloadCurrent();
	return false;
}

GameObject * ModuleLevelManager::CreateGameObject(GameObject * parent, const float3 & pos, const float3 & scale, const Quat & rot, const char * name)
{
	GameObject* ret = new GameObject(parent, name, pos, scale, rot);

	return ret;
}

GameObject * ModuleLevelManager::CreateGameObject(GameObject * parent)
{
	if (parent == nullptr)
		parent = root;

	return new GameObject(parent, "Unnamed");
}

GameObject * ModuleLevelManager::Duplicate(const GameObject * original)
{
	GameObject* ret = nullptr;

	if (original != nullptr)
	{
		Config save;
		save.AddArray("Game Objects");
		map<uint, uint> new_uids;
		original->Save(save, &new_uids);
	
		LoadGameObjects(save);
	}

	return ret;
}

void ModuleLevelManager::DestroyFlaggedGameObjects()
{
	// we never really have to remove root, but we remove all its childs
	if (root->flag_for_removal == true)
	{
		for (list<GameObject*>::iterator it = root->childs.begin(); it != root->childs.end(); ++it)
			(*it)->flag_for_removal = true;
		root->flag_for_removal = false;
	}

	// Find parent and cut connection
	if (root->RecursiveRemoveFlagged())
	{
		// Notify everybody that a GameObject has been destroyed
		// this gives the oportunity to other modules to Validate()
		// their pointers to GameObjects
		Event event(Event::gameobject_destroyed);
		App->BroadcastEvent(event);
	}
}

void ModuleLevelManager::LoadGameObjects(const Config & config)
{
	int count = config.GetArrayCount("Game Objects");
	map<GameObject*, uint> relations;
	for (int i = 0; i < count; ++i)
	{
		GameObject* go = CreateGameObject();
		go->Load(&config.GetArray("Game Objects", i), relations);
	}

	// Second pass to tide up the hierarchy
	for (map<GameObject*, uint>::iterator it = relations.begin(); it != relations.end(); ++it)
	{
		uint parent_id = it->second;
		GameObject* go = it->first;

		if (parent_id > 0)
		{
			GameObject* parent_go = Find(parent_id);
			if (parent_go != nullptr)
				go->SetNewParent(parent_go);
		}
	}

	// Reset all info about the level (this also fill in the quadtree)
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), true);
	root->RecursiveCalcBoundingBoxes();
	
	// Third pass: call OnStart on all new GameObjects
	for (map<GameObject*, uint>::iterator it = relations.begin(); it != relations.end(); ++it)
		it->first->OnStart();
}

bool ModuleLevelManager::Load(const char * file)
{
	bool ret = false;

	if (file != nullptr)
	{
		int len = strlen(file);

		char* buffer = nullptr;
		uint size = App->fs->Load(file, &buffer);

		if (buffer != nullptr && size > 0)
		{
			Config config(buffer);

			// Load level description
			Config desc(config.GetSection("Description"));
			name = desc.GetString("Name", "Unnamed level");
			App->camera->Load(&desc);

            Config lights = config.GetSection("Lights");
            ambient.Load(lights.GetSection("Ambient"));
            directional.Load(lights.GetSection("Directional"));

			LoadGameObjects(config);

		}

		RELEASE_ARRAY(buffer); 
	}

	return ret;
}

bool ModuleLevelManager::Save(const char * file)
{
	bool ret = true;

	Config save;

	// Add header info
	Config desc(save.AddSection("Description"));
	desc.AddString("Name", name.c_str());
	App->camera->Save(&desc);

    Config lights = save.AddSection("Lights");    

    ambient.Save(lights.AddSection("Ambient"));
    directional.Save(lights.AddSection("Directional"));

	// Serialize GameObjects recursively
	save.AddArray("Game Objects");

	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		(*it)->Save(save);

	// Finally save to file
	char* buf = nullptr;
	uint size = save.Save(&buf, "Level save file from EDU Engine");
	App->fs->Save(file, buf, size);
	RELEASE_ARRAY(buf);

	return ret;
}

void ModuleLevelManager::UnloadCurrent()
{
}

void ModuleLevelManager::RecursiveProcessEvent(GameObject * go, const Event & event) const
{
	switch (event.type)
	{
		case Event::EventType::play: go->OnPlay(); break;
		case Event::EventType::stop: go->OnStop(); break;
		case Event::EventType::pause: go->OnPause(); break;
		case Event::EventType::unpause: go->OnUnPause(); break;
		case Event::EventType::gameobject_destroyed: go->OnGoDestroyed(); break;
	}

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveProcessEvent(*it, event);
}

void ModuleLevelManager::RecursiveUpdate(GameObject * go, float dt) const
{
	go->OnUpdate(dt);

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveUpdate(*it, dt);
}

GameObject* ModuleLevelManager::RecursiveFind(uint uid, GameObject * go) const
{
	if (uid == go->GetUID())
		return go;

	GameObject* ret = nullptr;

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end() && ret == nullptr; ++it)
		ret = RecursiveFind(uid, *it);

	return ret;
}

GameObject * ModuleLevelManager::Validate(const GameObject * pointer) const
{
	if (pointer == root)
		return root;

	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		if (pointer == *it)
			return (GameObject *) pointer;

	return nullptr;
}

GameObject* ModuleLevelManager::CastRay(const LineSegment& segment, float& dist) const
{
	dist = inf;
	GameObject* candidate = nullptr;
	RecursiveTestRay(segment, dist, &candidate);
	return candidate;
}

void ModuleLevelManager::RecursiveTestRayBBox(const LineSegment & segment, float & dist, float3 & normal, GameObject ** best_candidate) const
{
	map<float, GameObject*> objects;
	quadtree.CollectIntersections(objects, segment);

	for (map<float, GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		// Look for meshes
		GameObject* go = it->second;
		if (go->HasComponent(Component::Types::Mesh) == true)
		{
			float closer = inf;
			*best_candidate = (GameObject*) go;
			dist = it->first;

			// let's find out the plane that hit the segment and fill in the normal
			for (int i = 0; i < 6; ++i)
			{
				Plane p(go->global_bbox.FacePlane(i));
				float d;
				if (p.Intersects(segment, &d))
				{
					if (d < closer)
						normal = p.normal;
				}
			}
		}
	}
}

void ModuleLevelManager::RecursiveTestRay(const LineSegment& segment, float& dist, GameObject** best_candidate) const
{
	map<float, GameObject*> objects;
	quadtree.CollectIntersections(objects, segment);

	for (map<float, GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		// Look for meshes, nothing else can be "picked" from the screen
		GameObject* go = it->second;
		vector<Component*> meshes;
		go->FindComponents(Component::Types::Mesh, meshes);

		if (meshes.size() > 0)
		{
			const ComponentMesh* cmesh = (const ComponentMesh*)meshes[0];
			const ResourceMesh* mesh = (ResourceMesh*) cmesh->GetResource();

			// test all triangles
			LineSegment segment_local_space(segment);
			segment_local_space.Transform(go->GetGlobalTransformation().Inverted());

			Triangle tri;
			for (uint i = 0; i < mesh->num_indices;)
			{
				tri.a = mesh->src_vertices[mesh->src_indices[i++]*3];
				tri.b = mesh->src_vertices[mesh->src_indices[i++]*3];
				tri.c = mesh->src_vertices[mesh->src_indices[i++]*3];

				float distance;
				float3 hit_point;
				if (segment_local_space.Intersects(tri, &distance, &hit_point))
				{
					if (distance < dist)
					{
						dist = distance;
						*best_candidate = (GameObject*) go;
					}
				}
			}
		}
	}
}

GameObject* ModuleLevelManager::CastRay(const Ray & ray, float& dist) const
{
	dist = inf;
	GameObject* candidate = nullptr;
	RecursiveTestRay(ray, dist, &candidate);
	return candidate;
}

GameObject * ModuleLevelManager::CastRayOnBoundingBoxes(const LineSegment & segment, float & dist, float3 & normal) const
{
	dist = inf;
	normal = float3::zero;
	GameObject* candidate = nullptr;
	RecursiveTestRayBBox(segment, dist, normal, &candidate);
	return candidate;
}

void ModuleLevelManager::RecursiveTestRay(const Ray& ray, float& dist, GameObject** best_candidate) const
{
	map<float, GameObject*> objects;
	quadtree.CollectIntersections(objects, ray);

	for (map<float, GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		// Look for meshes, nothing else can be "picked" from the screen
		GameObject* go = it->second;

		vector<Component*> meshes;
		go->FindComponents(Component::Types::Mesh, meshes);

		if (meshes.size() > 0)
		{
			const ComponentMesh* cmesh = (const ComponentMesh*)meshes[0];
			const ResourceMesh* mesh = (ResourceMesh*) cmesh->GetResource();

			// test all triangles
			Ray ray_local_space(ray);
			ray_local_space.Transform(go->GetGlobalTransformation().Inverted());
			ray_local_space.dir.Normalize();

			// Experiment using a TriangleMesh instead of raw triangles
			Triangle tri;
			for (uint i = 0; i < mesh->num_indices;)
			{
				tri.a = mesh->src_vertices[mesh->src_indices[i++]*3];
				tri.b = mesh->src_vertices[mesh->src_indices[i++]*3];
				tri.c = mesh->src_vertices[mesh->src_indices[i++]*3];
				// TODO I got a bug twice here, looks like a problem creating the triangle

				float distance;
				float3 hit_point;
				if (ray_local_space.Intersects(tri, &distance, &hit_point))
				{
					if (distance < dist)
					{
						dist = distance;
						*best_candidate = (GameObject*) go;
					}
				}
			}
		}
	}
}

void ModuleLevelManager::FindNear(const float3 & position, float radius, std::vector<GameObject*>& results) const
{
	quadtree.CollectIntersections(results, Sphere(position, radius));
}

