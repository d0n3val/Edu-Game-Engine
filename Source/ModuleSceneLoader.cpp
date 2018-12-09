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
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourceModel.h"
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

	unsigned flags = aiProcess_CalcTangentSpace | \
		aiProcess_GenSmoothNormals | \
		aiProcess_JoinIdenticalVertices | \
		aiProcess_ImproveCacheLocality | \
		aiProcess_LimitBoneWeights | \
		aiProcess_SplitLargeMeshes | \
		aiProcess_Triangulate | \
		aiProcess_GenUVCoords | \
		aiProcess_SortByPType | \
		aiProcess_FindDegenerates | \
		aiProcess_FindInvalidData | 
		0;
	
	aiString assimp_path(".");
	assimp_path.Append(full_path);

	const aiScene* scene = aiImportFile(assimp_path.data, flags);

	if (scene)
	{
        std::vector<UID> materials, meshes;
		GenerateMaterials(scene, full_path, materials);
		GenerateMeshes(scene, full_path, meshes);
        GenerateModel(scene, full_path, meshes, materials);

        // Generating prefab
		//GameObject* go = App->level->CreateGameObject(nullptr);
		//GenerateGameObjects(scene->mRootNode, go, meshes);

		aiReleaseImport(scene);

        // Generating prefab
		// Serialize GameObjects recursively
		//Config save;
		//save.AddArray("Game Objects");

		//for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			//(*it)->Save(save);

		// Finally save to file
		//char* buf = nullptr;
		//uint size = save.Save(&buf, "Prefab save file from EDU Engine");
		//bool ret = App->fs->SaveUnique(output, buf, size, LIBRARY_SCENE_FOLDER, "scene", "eduscene");
		//RELEASE_ARRAY(buf);

		// We can now safely remove the tree
		//go->Remove();

		//return ret;

		return true;
	}

	return false;
}

bool ModuleSceneLoader::AddModel(UID id)
{
    Resource* res = App->resources->Get(id);

    bool ok = res->GetType() == Resource::model;

    if(ok)
    {
        ResourceModel* model = static_cast<ResourceModel*>(res);
        model->LoadToMemory();

        std::vector<GameObject*> gos;
        gos.reserve(model->GetNumNodes());

        for(uint i=0, count = model->GetNumNodes(); ok && i< count; ++i)
        {
            const ResourceModel::Node& node = model->GetNode(i);

            GameObject* parent = i == 0 ? nullptr : gos[node.parent];
            GameObject* go = App->level->CreateGameObject(parent);

            go->SetLocalTransform(node.transform);
            go->name = node.name.c_str();

            if(node.mesh != 0)
            {
                ComponentMesh* mesh = new ComponentMesh(go);
                ok = mesh->SetResource(node.mesh);
                go->components.push_back(mesh);
            }

            if(ok && node.material != 0)
            {
                ComponentMaterial* material = new ComponentMaterial(go);
                ok = material->SetResource(node.material);
                go->components.push_back(material);
            }

            gos.push_back(go);
        }

        model->Release();

        if(!ok & !gos.empty())
        {
            gos.front()->Remove();
        }
    }



	return ok;
}

void ModuleSceneLoader::GenerateGameObjects(const aiNode* src, GameObject* dst, const std::vector<UID>& meshes)
{
    aiQuaternion quat;

	dst->SetLocalTransform(reinterpret_cast<const float4x4&>(src->mTransformation));
    dst->name = src->mName.C_Str();

    for(uint i=0; i< src->mNumMeshes; ++i)
    {
        ComponentMesh* geometry = new ComponentMesh(dst);

        geometry->SetResource(meshes[src->mMeshes[i]]);

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


void ModuleSceneLoader::GenerateMeshes(const aiScene* scene, const char* file, std::vector<UID>& meshes)
{
	meshes.reserve(scene->mNumMeshes);

	for(unsigned i=0; i < scene->mNumMeshes; ++i)
	{
        meshes.push_back(ResourceMesh::Import(scene->mMeshes[i], file)); 

		assert(meshes.back() != 0);
	}
}

UID ModuleSceneLoader::GenerateModel(const aiScene* scene, const char* file, const std::vector<UID>& meshes, std::vector<UID>& materials)
{
    return ResourceModel::Import(scene, meshes, materials, file);
}

