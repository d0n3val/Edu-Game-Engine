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

// Called before render is available
bool ModuleSceneLoader::Start(Config* config)
{
	LOG("Loading Scene Manager");
	bool ret = true;

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
bool ModuleSceneLoader::CleanUp()
{
	LOG("Freeing Scene Manager");

	// Clean scene data
	if(scene != nullptr)	// Unload Textures ?
		aiReleaseImport(scene);

	// detach log stream
	aiDetachAllLogStreams();


	return true;
}

void ModuleSceneLoader::RecursiveCreateGameObjects(const aiNode* node, GameObject* parent, const std::string& basePath)
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
					c_material->texture = App->tex->Load(App->tex->Import(tex->pcData, (tex->mHeight == 0) ? tex->mWidth : tex->mHeight*tex->mWidth));
				}
			}
			else
				c_material->texture = App->tex->Load(App->tex->Import(path.C_Str(), basePath.c_str()));
			LOG("->-> Added material component");
		}

		// Add mesh component
		ComponentMesh* c_mesh = (ComponentMesh*) child_go->CreateComponent(ComponentTypes::Geometry);
		c_mesh->SetMesh(App->meshes->Load(App->meshes->Import(mesh)));
		LOG("->-> Added mesh component");
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursiveCreateGameObjects(node->mChildren[i], go, basePath);
	}
}

bool ModuleSceneLoader::LoadScene(const char* file)
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
		RecursiveCreateGameObjects(scene->mRootNode, App->level->GetRoot(), basePath);

		// Release all info from assimp
		aiReleaseImport(scene);
		scene = nullptr;

		ret = true;
	}

	return ret;
}

void ModuleSceneLoader::LoadMetaData(aiMetadata * const meta)
{
	// iterate all metadata in this node
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
