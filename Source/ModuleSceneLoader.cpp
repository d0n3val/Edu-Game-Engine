#include "Globals.h"
#include "Application.h"
#include "ModuleSceneLoader.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "Config.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp.lib")

using namespace std;

ModuleSceneLoader::ModuleSceneLoader( bool start_enabled) : Module("Scene", start_enabled)
{
}

// Destructor
ModuleSceneLoader::~ModuleSceneLoader()
{}

// Called before render is available
bool ModuleSceneLoader::Init(Config* config)
{
	LOG("Loading Scene Manager");
	bool ret = true;
	struct aiLogStream stream;

	// Stream log messages to Debug window
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return ret;
}

// Called before quitting or switching levels
bool ModuleSceneLoader::CleanUp()
{
	LOG("Freeing Scene Manager");

	// detach log stream
	aiDetachAllLogStreams();

	return true;
}

void ModuleSceneLoader::RecursiveCreateGameObjects(const aiScene* scene, const aiNode* node, GameObject* parent, const string& basePath, const string& file)
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

	GameObject* go = App->level->CreateGameObject(parent, pos, scale, rot, node->mName.C_Str());

	LOG("Created new Game Object %s", go->name.c_str());

	// Load meta data
	LoadMetaData(node->mMetaData);

	// iterate all meshes in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		// Create a single game object per mesh
		//GameObject* child_go = CreateGameObject(go, aiMatrix4x4(), mesh->mName.C_Str());
		GameObject* child_go = App->level->CreateGameObject(go, float3::zero, float3::one, Quat::identity, mesh->mName.C_Str());
		LOG("-> Created new child Game Object %s", child_go->name.c_str());

		// Add material component if needed
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);

		if (numTextures >= 0)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			ComponentMaterial* c_material = (ComponentMaterial*) child_go->CreateComponent(ComponentTypes::Material);

			if (path.data[0] == '*')
			{
				uint n = atoi(&path.data[1]);
				// Embedded texture detected!
				if (n < scene->mNumTextures)
				{
					aiTexture* tex = scene->mTextures[n];
					UID id = App->resources->ImportBuffer(
						(const char*) tex->pcData, 
						(tex->mHeight == 0) ? tex->mWidth : tex->mHeight * tex->mWidth,
						Resource::texture
					);
					c_material->SetResource(id);
				}
			}
			else
			{
				string file(basePath.c_str());
				file += path.C_Str();
				c_material->SetResource(App->resources->ImportFile(file.c_str()));
			}
			LOG("->-> Added material component");
		}

		// Add mesh component
		ComponentMesh* c_mesh = (ComponentMesh*) child_go->CreateComponent(ComponentTypes::Geometry);
		c_mesh->SetResource(App->resources->ImportBuffer(mesh, 0, Resource::mesh, (basePath + file).c_str()));
		LOG("->-> Added mesh component");
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursiveCreateGameObjects(scene, node->mChildren[i], go, basePath, file);
	}
}

bool ModuleSceneLoader::Import(const char* full_path, std::string& output)
{
	bool ret = false;

	const aiScene* scene = aiImportFileEx(full_path, aiProcessPreset_TargetRealtime_MaxQuality, App->fs->GetAssimpIO());

	if (scene != nullptr)
	{
		// Generate base path
		string path, file;
		App->fs->SplitFilePath(full_path, &path, &file);

		// generate GameObjects for each mesh 
		GameObject* go = App->level->CreateGameObject();
		go->name = file;
		RecursiveCreateGameObjects(scene, scene->mRootNode, go, path, file);

		// Release all info from assimp
		aiReleaseImport(scene);

		// Serialize GameObjects recursively
		Config save;
		save.AddArray("Game Objects");

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		{
			Config child;
			(*it)->Save(child);
			save.AddArrayEntry(child);
		}

		// Finally save to file
		char* buf = nullptr;
		uint size = save.Save(&buf, "Prefab save file from EDU Engine");
		ret = App->fs->SaveUnique(output, buf, size, LIBRARY_SCENE_FOLDER, "scene", "eduscene");
		RELEASE(buf);

		// We can now safely remove the tree
		go->Remove();

		ret = true;
	}

	return ret;
}

void ModuleSceneLoader::LoadMetaData(aiMetadata * const meta)
{
	// TODO: Store this somehow ?
	if (meta != nullptr)
	{
		for (uint i = 0; i < meta->mNumProperties; ++i)
		{
			switch(meta->mValues[i].mType)
			{
				case AI_BOOL:
				{
					bool v;
					meta->Get(meta->mKeys[i].data, v);
					LOG("%s: %s", meta->mKeys[i].data, (v) ? "true" : "false");
				}	break;

				case AI_INT:
				{
					int v;
					meta->Get(meta->mKeys[i].data, v);
					LOG("%s: %i", meta->mKeys[i].data, v);
				}	break;

				case AI_UINT64:
				{
					unsigned long long v;
					meta->Get(meta->mKeys[i].data, v);
					LOG("%s: %u", meta->mKeys[i].data, v);
				}	break;

				case AI_FLOAT:
				{
					float v;
					meta->Get(meta->mKeys[i].data, v);
					LOG("%s: %.3f", meta->mKeys[i].data, v);
				}	break;

				case AI_AISTRING:
				{
					aiString v;
					meta->Get(meta->mKeys[i].data, v);
					LOG("%s: %s", meta->mKeys[i].data, v.C_Str());
				}	break;

				case AI_AIVECTOR3D:
				{
					aiVector3D v;
					meta->Get(meta->mKeys[i], v);
					LOG("%s: %.3f,%.3f,%.3f", meta->mKeys[i].data, v.x, v.y, v.z);
				}	break;	  
			}
		}
	}
}
