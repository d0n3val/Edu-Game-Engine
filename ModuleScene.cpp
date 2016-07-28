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
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc130-mt.lib")

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
	root = new GameObject();

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
	aiMatrix4x4 transform = node->mTransformation;
	aiTransposeMatrix4(&transform);

	GameObject* go = new GameObject(node->mName.C_Str());
	parent->childs.push_back(go);
	memcpy(go->transform.M, &transform, sizeof(float) * 16);
	LOG("Created new Game Object %s", go->name.c_str());

	// iterate all meshes in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		// Create a single game object per mesh
		GameObject* child_go = new GameObject(mesh->mName.C_Str());
		go->childs.push_back(child_go);
		LOG("-> Created new child Game Object %s", child_go->name.c_str());

		// Add material component if needed
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);

		if (numTextures >= 0)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

			ComponentMaterial* c_material = new ComponentMaterial(child_go);
			child_go->components.push_back(c_material);
			c_material->material_id = App->tex->Load(path.C_Str(), basePath.c_str());
			LOG("->-> Added material component");
		}

		// Add mesh component
		ComponentMesh* c_mesh = new ComponentMesh(child_go);
		child_go->components.push_back(c_mesh);
		c_mesh->mesh_data = App->meshes->Load(mesh);
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

void ModuleScene::Draw() const
{
	RecursiveDrawGameObjects(root);
}

void ModuleScene::RecursiveDrawGameObjects(const GameObject* go) const
{
	// push this matrix before drawing
	glPushMatrix();
	glMultMatrixf((float*)&go->transform);

	glBindTexture(GL_TEXTURE_2D, 0);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Material)
		{
			ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
			glBindTexture(GL_TEXTURE_2D, cmaterial->material_id);
		}
	}

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Geometry)
		{
			ComponentMesh* cmesh = (ComponentMesh*) (*it);
			const Mesh* mesh = cmesh->mesh_data;

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

	// Recursive call to all childs keeping matrices
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDrawGameObjects(*it);

	// pop this matrix before leaving this node
	glPopMatrix();		
}
