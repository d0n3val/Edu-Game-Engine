#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleFileSystem.h"
#include "ModuleMeshes.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Config.h"
#include "ModuleRenderer3D.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "OpenGL.h"
#include "Primitive.h"
#include "ComponentCamera.h"

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
		}

		RELEASE(buffer); // since we are not buffering the file, we can safely remove it
	}

	return ret;
}

bool ModuleLevelManager::Save(const char * file)
{
	bool ret = true;

	Config save;
	save.CreateEmpty();

	// Add header info
	Config desc(save.AddSection("Description"));
	desc.AddString("Name", name.c_str());

	// Serialize GameObjects recursively
	

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

	if (App->renderer3D->active_camera != nullptr && go->global_bbox.IsFinite() == true)
	{
		if (App->renderer3D->active_camera->frustum.Intersects(go->global_bbox) == false)
		{
			go->visible = false;
			return;
		}
		go->visible = true;
	}

	// push this matrix before drawing
	glPushMatrix();
	glMultMatrixf(go->GetOpenGLGlobalTranform());

	glBindTexture(GL_TEXTURE_2D, 0);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == ComponentTypes::Material)
		{
			ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
			glBindTexture(GL_TEXTURE_2D, cmaterial->material_id);
		}
	}

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == ComponentTypes::Geometry)
		{
			ComponentMesh* cmesh = (ComponentMesh*) (*it);
			const Mesh* mesh = cmesh->GetMesh();

			if (mesh->vbo_normals > 0)
			{
				glEnable(GL_LIGHTING);
				//glEnableClientState(GL_NORMAL_ARRAY);
				
				glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);
				glNormalPointer(3, GL_FLOAT, NULL);
			}
			else
				glDisable(GL_LIGHTING);
			  
			glEnableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_vertices);
			glVertexPointer(3, GL_FLOAT, 0, NULL);

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
			glTexCoordPointer(3, GL_FLOAT, 0, NULL);
			  
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_indices);
			glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY );
		}
	}

	// we no longer need this matrix (all global transforms are already calculated)
	glPopMatrix();		

	// Recursive call to all childs keeping matrices
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDrawGameObjects(*it);
}