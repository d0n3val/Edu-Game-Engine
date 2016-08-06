#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "Config.h"
#include "OpenGL.h"
#include "Primitive.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp.lib")

using namespace std;

ModuleScene::ModuleScene( bool start_enabled) : Module("Scene", start_enabled)
{}

// Destructor
ModuleScene::~ModuleScene()
{}

// Called before render is available
bool ModuleScene::Init(Config* config)
{
	LOG("Loading Scene Manager");
	bool ret = true;
	struct aiLogStream stream;

	// Stream log messages to Debug window
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	// create an empty game object to be the root of everything
	root = new GameObject("root");

	// Load conf
	if (config != nullptr && config->IsValid() == true)
	{
		uint i = 0;
		const char* str = config->GetString("ToLoad", nullptr, i);

		while (str != nullptr)
		{
			LoadScene(str);
			str = config->GetString("ToLoad", nullptr, ++i);
		}
	}

	return ret;
}

bool ModuleScene::Start(Config * config)
{
	// Pre-calculate all transformations and bboxes
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), true);
	bool did_recalc;
	root->RecursiveCalcBoundingBoxes(did_recalc);
	return true;
}

update_status ModuleScene::PreUpdate(float dt)
{
	// Update transformations tree for this frame
	root->RecursiveCalcGlobalTransform(root->GetLocalTransform(), false);
	bool did_recalc;
	root->RecursiveCalcBoundingBoxes(did_recalc);

	return UPDATE_CONTINUE;
}

// Called before quitting or switching levels
bool ModuleScene::CleanUp()
{
	LOG("Freeing Scene Manager");

	// Clean scene data
	if(scene != nullptr)	// Unload Textures ?
		aiReleaseImport(scene);

	// detach log stream
	aiDetachAllLogStreams();

	// destructor should trigger a recursive destruction of the whole tree
	RELEASE(root);

	return true;
}

void ModuleScene::RecursiveCreateGameObjects(const aiNode* node, GameObject* parent, const std::string& basePath)
{
	aiVector3D translation;
	aiVector3D scaling;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scaling, rotation, translation);

	float3 pos(translation.x, translation.y, translation.z);
	float3 scale(scaling.x, scaling.y, scaling.z);
	Quat rot(rotation.x, rotation.y, rotation.z, rotation.w);

	float4x4 m(rot, pos);
	m.Scale(scale);

	GameObject* go = CreateGameObject(parent, pos, scale, rot, node->mName.C_Str());

	LOG("Created new Game Object %s", go->name.c_str());

	// Load meta data
	LoadMetaData(node->mMetaData);

	// iterate all meshes in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		// Create a single game object per mesh
		//GameObject* child_go = CreateGameObject(go, aiMatrix4x4(), mesh->mName.C_Str());
		GameObject* child_go = CreateGameObject(go, float3::zero, float3::one, Quat::identity, mesh->mName.C_Str());
		LOG("-> Created new child Game Object %s", child_go->name.c_str());

		// Add material component if needed
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);

		if (numTextures >= 0)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

			ComponentMaterial* c_material = (ComponentMaterial*) child_go->CreateComponent(ComponentTypes::Material);
			c_material->material_id = App->tex->Load(path.C_Str(), basePath.c_str());
			LOG("->-> Added material component");
		}

		// Add mesh component
		ComponentMesh* c_mesh = (ComponentMesh*) child_go->CreateComponent(ComponentTypes::Geometry);
		c_mesh->SetMesh(App->meshes->Load(mesh));
		LOG("->-> Added mesh component");
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursiveCreateGameObjects(node->mChildren[i], go, basePath);
	}
}

bool ModuleScene::LoadScene(const char* file)
{
	bool ret = false;

	scene = aiImportFileEx(file, aiProcessPreset_TargetRealtime_MaxQuality, App->fs->GetAssimpIO());

	if (scene != nullptr)
	{
		// Generate base path
		std::string basePath(file);
		size_t pos = basePath.find_last_of("\\/");
		if (pos != string::npos)
			basePath.erase(pos + 1, string::npos);

		// generate GameObjects for each mesh 
		RecursiveCreateGameObjects(scene->mRootNode, root, basePath);

		// Release all info from assimp
		aiReleaseImport(scene);
		scene = nullptr;

		ret = true;
	}

	return ret;
}

void ModuleScene::LoadMetaData(aiMetadata * const meta)
{
	// iterate all metadata in this node
	if (meta != nullptr)
	{
		for (uint i = 0; i < meta->mNumProperties; ++i)
		{
			LOG("Key: %s", meta->mKeys[i]);
			switch(meta->mValues[i].mType)
			{
				case AI_BOOL:
				{
					bool v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a bool with %s", (v) ? "true" : "false");
				}	break;

				case AI_INT:
				{
					int v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a int with %i", v);
				}	break;

				case AI_UINT64:
				{
					unsigned long long v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a uint64 with %u", v);
				}	break;

				case AI_FLOAT:
				{
					float v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a float with %.3f", v);
				}	break;

				case AI_AISTRING:
				{
					aiString v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a string with %s", v.C_Str());
				}	break;

				case AI_AIVECTOR3D:
				{
					aiVector3D v;
					meta->Get(meta->mKeys[i], v);
					LOG("Is a vector3 with %.3f, %.3f, %.3f", v.x, v.y, v.z);
				}	break;	  
			}
		}
	}
}

void ModuleScene::Draw() const
{
	RecursiveDrawGameObjects(root);
}

const GameObject * ModuleScene::GetRoot() const
{
	return root;
}

GameObject * ModuleScene::GetRoot()
{
	return root;
}

GameObject * ModuleScene::CreateGameObject(GameObject * parent, const float3 & pos, const float3 & scale, const Quat & rot, const char * name)
{
	if (parent == nullptr)
		parent = App->scene->GetRoot();

	GameObject* ret = new GameObject(name, pos, scale, rot);
	parent->AddChild(ret);

	return ret;
}

void ModuleScene::RecursiveDrawGameObjects(const GameObject* go) const
{
	// Avoid inactive gameobjects
	if (go->IsActive() == false)
		return;

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
