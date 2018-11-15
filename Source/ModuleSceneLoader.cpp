#include "Globals.h"
#include "Application.h"
#include "ModuleSceneLoader.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "GameObject.h"
#include "ComponentGeometry.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentBone.h"
#include "Config.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#include "mmgr/mmgr.h"
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

bool ModuleSceneLoader::Start(Config * config)
{
	string t;
	//Import("/Assets/Animation/Ethan/Ethan.fbx", t);
	return true;
}

// Called before quitting or switching levels
bool ModuleSceneLoader::CleanUp()
{
	LOG("Freeing Scene Manager");

	// detach log stream
	aiDetachAllLogStreams();

	return true;
}

bool ModuleSceneLoader::Import(const char* full_path, std::string& output)
{
	GameObject* node = nullptr;

	unsigned flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph;

	aiString assimp_path(".");
	assimp_path.Append(full_path);

	const aiScene* scene = aiImportFile(assimp_path.data,  flags);

	if (scene)
	{
        std::vector<UID> materials, meshes;
		GenerateMaterials(scene, full_path, materials);
		GenerateMeshes(scene, full_path, materials, meshes);
		GenerateGameObjects(scene->mRootNode, App->level->CreateGameObject(nullptr), meshes);

		aiReleaseImport(scene);

		return true;
	}

	return false;
}


void ModuleSceneLoader::GenerateGameObjects(const aiNode* src, GameObject* dst, const std::vector<UID>& meshes)
{
    aiQuaternion quat;
	dst->SetLocalTransform(reinterpret_cast<const float4x4&>(src->mTransformation));
    dst->name = src->mName.C_Str();

    if(src->mNumMeshes > 0)
    {
        ComponentGeometry* geometry = new ComponentGeometry(dst);
        geometry->Initialize(&meshes[0], src->mMeshes, src->mNumMeshes);

        dst->components.push_back(geometry);
    }


	for(unsigned i=0; i < src->mNumChildren; ++i)
	{
		GenerateGameObjects(src->mChildren[i], App->level->CreateGameObject(dst), meshes);
	}
}

void ModuleSceneLoader::GenerateMaterials(const aiScene* scene, const char* file, std::vector<UID>& materials)
{
	materials.reserve(scene->mNumMaterials);

	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
        materials.push_back(ResourceMaterial::Import(scene->mMaterials[i], file));
		assert(materials.back() != 0);
	}
}

void ModuleSceneLoader::GenerateMeshes(const aiScene* scene, const char* file, const std::vector<UID>& materials, std::vector<UID>& meshes)
{
	meshes.reserve(scene->mNumMeshes);

	for(unsigned i=0; i < scene->mNumMeshes; ++i)
	{
        meshes.push_back(ResourceMesh::Import(scene->mMeshes[i],  materials[scene->mMeshes[i]->mMaterialIndex], file)); 
		assert(meshes.back() != 0);
	}
}
