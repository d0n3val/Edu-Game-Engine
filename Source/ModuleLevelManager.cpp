#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleFileSystem.h"
#include "ModuleSceneLoader.h"
#include "GameObject.h"
#include "Config.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
#include "DebugDraw.h"
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
void ModuleLevelManager::DrawDebug()
{
	if (draw_quadtree == true)
	{
		vector<const QuadtreeNode*> boxes;
		quadtree.CollectBoxes(boxes);

		for (vector<const QuadtreeNode*>::const_iterator it = boxes.begin(); it != boxes.end(); ++it)
			DebugDraw((*it)->box, Yellow);

		vector<GameObject*> objects;
		quadtree.CollectObjects(objects);

		for (vector<GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
			DebugDraw((*it)->global_bbox.MinimalEnclosingAABB(), Red);
	}
}

const GameObject * ModuleLevelManager::GetRoot() const
{
	return root;
}

GameObject * ModuleLevelManager::GetRoot()
{
	return root;
}

bool ModuleLevelManager::CreateNewEmpty(const char * name)
{
	UnloadCurrent();
	return false;
}

void ModuleLevelManager::Draw() const
{
	RecursiveDrawGameObjects(root);
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

			int count = config.GetArrayCount("Game Objects");
			map<int, GameObject*> relations;
			for (int i = 0; i < count; ++i)
			{
				GameObject* go = CreateGameObject();
				go->Load(&config.GetArray("Game Objects", i), relations);
			}

			// Second pass to tide up the hierarchy
			for (map<int, GameObject*>::iterator it = relations.begin(); it != relations.end(); ++it)
			{
				int my_id = it->first;
				GameObject* go = it->second;
				int parent_id = go->serialization_id;

				if (parent_id > 0)
				{
					GameObject* parent_go = relations[parent_id];
					go->SetNewParent(parent_go);
					//parent_go->childs.push_back(go);
				}
			}

			// Reset all info about the level
			root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), true);
			root->RecursiveCalcBoundingBoxes();
			
			// Fill in the quadtree
			quadtree.Clear();
			quadtree.SetBoundaries(AABB(float3(-500,0,-500), float3(500,50,500)));
			for (map<int, GameObject*>::iterator it = relations.begin(); it != relations.end(); ++it)
				quadtree.Insert(it->second);

			// Third pass: call OnStart on all new GameObjects
			for (map<int, GameObject*>::iterator it = relations.begin(); it != relations.end(); ++it)
				it->second->OnStart();

		}

		RELEASE_ARRAY(buffer); 
	}

	return ret;
}

bool ModuleLevelManager::Save(const char * file)
{
	bool ret = true;
	int file_uid = 1;

	Config save;

	// Add header info
	Config desc(save.AddSection("Description"));
	desc.AddString("Name", name.c_str());
	App->camera->Save(&desc);

	// Serialize GameObjects recursively
	save.AddArray("Game Objects");

	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
	{
		(*it)->Save(save, file_uid, root);
	}

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

// TODO that should be a combination of the UID of the scene resource + the local serialization_id
GameObject * ModuleLevelManager::Find(uint serialization_id, const GameObject* from) const
{
	if (serialization_id > 0)
	{
		if (from->serialization_id == serialization_id)
			return (GameObject *) from;

		for (list<GameObject*>::const_iterator it = from->childs.begin(); it != from->childs.end(); ++it)
			return Find(serialization_id, *it);
	}

	return nullptr;
}

void ModuleLevelManager::RecursiveDrawGameObjects(const GameObject* go) const
{
	vector<GameObject*> objects;

	// we do frustum culling or not ?
	ComponentCamera* cam = App->renderer3D->active_camera;

	if (cam != nullptr && cam->frustum_culling == true)
		quadtree.CollectIntersections(objects, cam->frustum);
	else
		quadtree.CollectObjects(objects);

	for (vector<GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
		if((*it)->IsActive())
			(*it)->Draw();
	/*

	// Avoid inactive gameobjects
	if (go->IsActive() == false)
		return;

	ComponentCamera* cam = App->renderer3D->active_camera;

	if (cam != nullptr && cam->frustum_culling == true && go->global_bbox.IsFinite() == true)
	{
		if (cam->frustum.Intersects(go->global_bbox) == false)
		{
			go->visible = false;
			return;
		}
	}

	go->Draw();

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDrawGameObjects(*it);
		*/
}

void ModuleLevelManager::RecursiveProcessEvent(GameObject * go, const Event & event) const
{
	switch (event.type)
	{
		case Event::EventType::play: go->OnPlay(); break;
		case Event::EventType::stop: go->OnStop(); break;
		case Event::EventType::pause: go->OnPause(); break;
		case Event::EventType::unpause: go->OnUnPause(); break;
	}

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveProcessEvent(*it, event);
}

void ModuleLevelManager::RecursiveDebugDrawGameObjects(const GameObject* go) const
{
	go->OnDebugDraw();

	// Recursive call to all childs keeping matrices
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDebugDrawGameObjects(*it);
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

void ModuleLevelManager::RecursiveTestRay(const LineSegment& segment, float& dist, GameObject** best_candidate) const
{
	map<float, GameObject*> objects;
	quadtree.CollectIntersections(objects, segment);

	for (map<float, GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		// Look for meshes, nothing else can be "picked" from the screen
		GameObject* go = it->second;
		vector<Component*> meshes;
		go->FindComponents(Component::Types::Geometry, meshes);

		if (meshes.size() > 0)
		{
			const ComponentMesh* cmesh = (const ComponentMesh*)meshes[0];
			const ResourceMesh* mesh = cmesh->deformable ? cmesh->deformable : (ResourceMesh*) cmesh->GetResource();

			// test all triangles
			LineSegment segment_local_space(segment);
			segment_local_space.Transform(go->GetGlobalTransformation().Inverted());

			Triangle tri;
			for (uint i = 0; i < mesh->num_indices;)
			{
				tri.a.Set(&mesh->vertices[mesh->indices[i++]*3]);
				tri.b.Set(&mesh->vertices[mesh->indices[i++]*3]);
				tri.c.Set(&mesh->vertices[mesh->indices[i++]*3]);

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

void ModuleLevelManager::RecursiveTestRay(const Ray& ray, float& dist, GameObject** best_candidate) const
{
	map<float, GameObject*> objects;
	quadtree.CollectIntersections(objects, ray);

	for (map<float, GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		// Look for meshes, nothing else can be "picked" from the screen
		GameObject* go = it->second;

		vector<Component*> meshes;
		go->FindComponents(Component::Types::Geometry, meshes);

		if (meshes.size() > 0)
		{
			const ComponentMesh* cmesh = (const ComponentMesh*)meshes[0];
			const ResourceMesh* mesh = cmesh->deformable ? cmesh->deformable : (ResourceMesh*) cmesh->GetResource();

			// test all triangles
			Ray ray_local_space(ray);
			ray_local_space.Transform(go->GetGlobalTransformation().Inverted());
			ray_local_space.dir.Normalize();

			// Experiment using a TriangleMesh instead of raw triangles
			Triangle tri;
			for (uint i = 0; i < mesh->num_indices;)
			{
				tri.a.Set(&mesh->vertices[mesh->indices[i++]*3]);
				tri.b.Set(&mesh->vertices[mesh->indices[i++]*3]);
				tri.c.Set(&mesh->vertices[mesh->indices[i++]*3]);
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

