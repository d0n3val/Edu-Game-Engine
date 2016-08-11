#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleFileSystem.h"
#include "ModuleSceneLoader.h"
#include "GameObject.h"
#include "Config.h"
#include "ModuleRenderer3D.h"
#include "ComponentCamera.h"
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
	root = new GameObject("root");

	return ret;
}

bool ModuleLevelManager::Start(Config * config)
{
	// Pre-calculate all transformations and bboxes
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), true);
	bool did_recalc;
	root->RecursiveCalcBoundingBoxes(did_recalc);
	return true;
}

update_status ModuleLevelManager::PreUpdate(float dt)
{
	DestroyFlaggedGameObjects();
	// Update transformations tree for this frame
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), false);
	bool did_recalc;
	root->RecursiveCalcBoundingBoxes(did_recalc);

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
	if (parent == nullptr)
		parent = root;

	GameObject* ret = new GameObject(name, pos, scale, rot);
	parent->AddChild(ret);

	return ret;
}

GameObject * ModuleLevelManager::CreateGameObject(GameObject * parent)
{
	if (parent == nullptr)
		parent = root;

	GameObject* ret = new GameObject("Unnamed");
	parent->AddChild(ret);
	return ret;
}

void ModuleLevelManager::RecursiveRemove(GameObject * go)
{
	if (go == nullptr)
		root->Remove();
	else
		go->Remove();
}

void ModuleLevelManager::DestroyFlaggedGameObjects()
{
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

		if (buffer != nullptr)
		{
			Config config(buffer);
			if (config.IsValid())
				RecursiveRemove(root);

			// Load level description
			Config desc(config.GetSection("Desscription"));
			name = desc.GetString("Name", "Unnamed level");

			int count = config.GetArrayCount("Game Objects");
			for (int i = 0; i < count; ++i)
			{
				GameObject* go = CreateGameObject();
				go->Load(&config.GetArray("Game Objects", i));
			}

			root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), true);
			bool did_recalc;
			root->RecursiveCalcBoundingBoxes(did_recalc);
		}

		RELEASE(buffer); 
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

	// Serialize GameObjects recursively
	save.AddArray("Game Objects");

	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
	{
		Config child;
		(*it)->Save(child);
		save.AddArrayEntry(child);
	}

	// Finally save to file
	char* buf = nullptr;
	uint size = save.Save(&buf, "Level save file from EDU Engine");
	App->fs->Save(file, buf, size);
	RELEASE(buf);

	return ret;
}

void ModuleLevelManager::UnloadCurrent()
{
}

void ModuleLevelManager::RecursiveDrawGameObjects(const GameObject* go) const
{
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

	// Recursive call to all childs keeping matrices
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDrawGameObjects(*it);
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
