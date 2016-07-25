#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleMeshes.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include <gl/GL.h>
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc130-mt.lib")

using namespace std;

ModuleScene::ModuleScene( bool start_enabled) : Module( start_enabled)
{}

// Destructor
ModuleScene::~ModuleScene()
{}

// Called before render is available
bool ModuleScene::Init()
{
	LOG("Loading Scene Manager");
	bool ret = true;
	struct aiLogStream stream;

	// Stream log messages to Debug window
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	// create an empty game object to be the root of everything
	root = new GameObject();

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
	if (scene != nullptr) // Unload all textures ?
		aiReleaseImport(scene);

	scene = aiImportFile(file, aiProcessPreset_TargetRealtime_Quality);

	if (scene != nullptr)
	{
		// Generate base path
		std::string basePath(file);
		size_t pos = basePath.find_last_of("\\/");
		if (pos != string::npos)
			basePath.erase(pos + 1, string::npos);


		// generate GameObjects for each mesh 
		RecursiveCreateGameObjects(scene->mRootNode, root, basePath);
	}

	aiReleaseImport(scene);
	scene = nullptr;

	return scene != nullptr;
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

			// iterate all faces
			for (uint k = 0; k < mesh->num_faces; ++k)
			{
				const Face* face = &mesh->faces[k];

				mesh->normals ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
				mesh->colors ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);

				glBegin(GL_POLYGON);

				// iterate all indices
				for (uint j = 0; j < face->num_indices; ++j)
				{
					int index = face->indices[j] * 3;

					if(mesh->texture_coords != nullptr)
						glTexCoord2f(mesh->texture_coords[index], mesh->texture_coords[index+1]);

					if(mesh->normals != nullptr) 
						glNormal3fv(&mesh->normals[index]);
					  
					glVertex3fv(&mesh->vertices[index]);
				}

				glEnd();
			}
		}
	}

	// Recursive call to all childs keeping matrices
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveDrawGameObjects(*it);

	glPopMatrix();		

	// pop this matrix before leaving this node
}
