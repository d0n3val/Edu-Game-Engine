#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleMeshes.h"
#include "GameObject.h"
#include "ComponentMesh.h"
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

void ModuleScene::RecursiveCreateGameObjects(const aiNode* node, GameObject* parent)
{
	GameObject* go = new GameObject();
	parent->childs.push_back(go);

	// set GO transformation
	aiMatrix4x4 transform = node->mTransformation;
	aiTransposeMatrix4(&transform);
	memcpy(go->transform.M, &transform, sizeof(float) * 16);

	LOG("Created new Game Object");
	// iterate all meshes in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		// Add mesh component
		ComponentMesh* c_mesh = new ComponentMesh(go);
		go->components.push_back(c_mesh);
		c_mesh->mesh_data = App->meshes->Load(mesh);
		LOG("... Added mesh component");
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursiveCreateGameObjects(node->mChildren[i], go);
	}
}

bool ModuleScene::LoadScene(const char* file)
{
	if (scene != nullptr) // Unload all textures ?
		aiReleaseImport(scene);

	scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	// generate GameObjects based on scene data
	RecursiveCreateGameObjects(scene->mRootNode, root);

	// Load textures
	if (scene != nullptr)
	{
		for (uint i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* material = scene->mMaterials[i];
			uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
			aiString path;

			for (uint k = 0; k < numTextures; ++k)
			{
				material->GetTexture(aiTextureType_DIFFUSE, k, &path);

				std::string texPath(file);
				size_t pos = texPath.find_last_of("\\/");
				if (pos != string::npos)
					texPath.erase(pos + 1, string::npos);

				App->tex->Load(path.C_Str(), texPath.c_str());
			}
		}
	}

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

void ModuleScene::PrepareMaterial(const aiMaterial* mtl) const
{
	aiString texPath;	//contains filename of texture

	mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);

	GLuint id = App->tex->GetId(texPath.C_Str());
	glBindTexture(GL_TEXTURE_2D, id);
}
