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

void ModuleSceneLoader::RecursiveCreateGameObjects(const aiScene* scene, const aiNode* node, GameObject* parent, const string& basePath, const string& file)
{
#if 0 
    // \todo: 
	static string name;
	name = (node->mName.length > 0) ? node->mName.C_Str() : "Unnamed";
	aiVector3D translation;
	aiVector3D scaling;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scaling, rotation, translation);

	float3 pos(translation.x, translation.y, translation.z);
	float3 scale(scaling.x, scaling.y, scaling.z);
	Quat rot(rotation.x, rotation.y, rotation.z, rotation.w);

	// Name analysis to handle FBX dummy nodes
	// check bottom of http://g3d.cs.williams.edu/g3d/G3D10/assimp.lib/code/FBXImportSettings.h
	static const char* dummies[5] = {
		"$AssimpFbx$_PreRotation", "$AssimpFbx$_Rotation", "$AssimpFbx$_PostRotation",
		"$AssimpFbx$_Scaling", "$AssimpFbx$_Translation"};

	for (int i = 0; i < 5; ++i)
	{
		if (name.find(dummies[i]) != string::npos && node->mNumChildren == 1)
		{
			node = node->mChildren[0];

			node->mTransformation.Decompose(scaling, rotation, translation);
			// accumulate transform
			pos += float3(translation.x, translation.y, translation.z);
			scale = float3(scale.x * scaling.x, scale.y * scaling.y, scale.z * scaling.z);
			rot = rot * Quat(rotation.x, rotation.y, rotation.z, rotation.w);

			name = node->mName.C_Str();
			i = -1; // start over!
		}
	}
	// ---


	float4x4 m(rot, pos);
	m.Scale(scale);

	GameObject* go = App->level->CreateGameObject(parent, pos, scale, rot, name.c_str());

	relations[node] = go;

	// Load meta data
	LoadMetaData(node->mMetaData);

	// iterate all meshes in this node
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		GameObject* child_go = nullptr;

		if (node->mNumMeshes > 1)
		{
			// If we have sub_meshes, create a single game object per mesh
			name = mesh->mName.C_Str();
			if (name.length() == 0)
			{
				name = node->mName.C_Str();
				name += "_submesh";
			}
			child_go = App->level->CreateGameObject(go, float3::zero, float3::one, Quat::identity, name.c_str());
		}
		else
			child_go = go;

		// Add material component if needed
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);

		if (numTextures >= 0)
		{
			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			ComponentMaterial* c_material = (ComponentMaterial*)child_go->CreateComponent(Component::Types::Material);

			if (path.data[0] == '*')
			{
				uint n = atoi(&path.data[1]);
				// Embedded texture detected!
				if (n < scene->mNumTextures)
				{
					aiTexture* tex = scene->mTextures[n];
					UID id = App->resources->ImportBuffer(
						(const char*)tex->pcData,
						(tex->mHeight == 0) ? tex->mWidth : tex->mHeight * tex->mWidth,
						Resource::texture
					);
					c_material->SetResource(id);
				}
			}
			else
			{
				string file(basePath);
				file += path.C_Str();
				if (App->fs->Exists(file.c_str()) == false)
				{
					// try extracting the file from path
					string extracted;
					App->fs->SplitFilePath(path.C_Str(), nullptr, &extracted);
					file = basePath;
					file += extracted;
					LOG("File [%s%s] does not exist, trying [%s]", basePath.c_str(), path.C_Str(), file.c_str());
				}
				c_material->SetResource(App->resources->ImportFile(file.c_str()));
			}
			LOG("->-> Added material component");
		}

		// Add mesh component
		ComponentMesh* c_mesh = (ComponentMesh*)child_go->CreateComponent(Component::Types::Geometry);
		c_mesh->SetResource(App->resources->ImportBuffer(mesh, 0, Resource::mesh, (basePath + file).c_str()));
		LOG("->-> Added mesh component");

		// If we have bones keep them for later
		if (mesh->HasBones() == true)
		{
			int num = mesh->mNumBones;
			for (int i = 0; i < num; ++i)
			{
				bones[mesh->mBones[i]->mName.C_Str()] = mesh->mBones[i];
				mesh_bone[mesh->mBones[i]] = c_mesh->GetResourceUID();
			}
		}
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
		RecursiveCreateGameObjects(scene, node->mChildren[i], go, basePath, file);
#endif
}

void ModuleSceneLoader::RecursiveProcessBones(const aiScene * scene, const aiNode * node)
{
	// We need to find if this node it supposed to hold a bone
	// for that we will look for all the other meshes and look
	// if there is a mach in the name
	map<string, aiBone*>::iterator it = bones.find(node->mName.C_Str());

	if(it != bones.end())
	{
		aiBone* bone = it->second; 

		GameObject* go = relations[node];
		ComponentBone* c_bone = (ComponentBone*) go->CreateComponent(Component::Types::Bone);

		UID uid = App->resources->ImportBuffer(bone, (uint) mesh_bone[bone], Resource::bone, bone->mName.C_Str());
		c_bone->SetResource(uid);
		imported_bones[node->mName.C_Str()] = uid;
		LOG("->-> Added Bone component and created bone resource");
	}

	// recursive call to generate the rest of the scene tree
	for (uint i = 0; i < node->mNumChildren; ++i)
		RecursiveProcessBones(scene, node->mChildren[i]);
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
		bones.clear();
		relations.clear();
		GameObject* go = App->level->CreateGameObject();
		go->name = file;
		RecursiveCreateGameObjects(scene, scene->mRootNode, go, path, file);

		// Do a second pass to process bones
		RecursiveProcessBones(scene, scene->mRootNode);

		// Now search for animations
		ImportAnimations(scene, full_path);

		// Release all info from assimp
		aiReleaseImport(scene);

		// Serialize GameObjects recursively
		Config save;
		save.AddArray("Game Objects");

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			(*it)->Save(save);

		// Finally save to file
		char* buf = nullptr;
		uint size = save.Save(&buf, "Prefab save file from EDU Engine");
		ret = App->fs->SaveUnique(output, buf, size, LIBRARY_SCENE_FOLDER, "scene", "eduscene");
		RELEASE_ARRAY(buf);

		// We can now safely remove the tree
		go->Remove();

		ret = true;
	}

	return ret;
}

bool ModuleSceneLoader::ImportNew(const char* full_path, std::string& output)
{
	GameObject* node = nullptr;

	unsigned flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph;

	const aiScene* scene = aiImportFile(full_path,  flags);

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
		GameObject* child = App->level->CreateGameObject(dst);
		dst->childs.push_back(child);
		
		GenerateGameObjects(src->mChildren[i], child, meshes);
	}
}

void ModuleSceneLoader::GenerateMaterials(const aiScene* scene, const char* file, std::vector<UID>& materials)
{
	materials.reserve(scene->mNumMaterials);

	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
        materials.push_back(ResourceMaterial::Import(scene->mMaterials[i], file));
	}
}

void ModuleSceneLoader::GenerateMeshes(const aiScene* scene, const char* file, const std::vector<UID>& materials, std::vector<UID>& meshes)
{
	meshes.reserve(scene->mNumMeshes);

	for(unsigned i=0; i < scene->mNumMeshes; ++i)
	{
        meshes.push_back(ResourceMesh::Import(scene->mMeshes[i],  materials[i], file)); 
		assert(meshes.back() != 0);
	}
}

UID ModuleSceneLoader::FindBoneFromLastImport(const char * name) const
{
	if (imported_bones.find(name) != imported_bones.end())
		return imported_bones.at(name);

	return 0;
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

void ModuleSceneLoader::ImportAnimations(const aiScene * scene, const char* full_path)
{
	for (uint i = 0; i < scene->mNumAnimations; ++i)
	{
		const aiAnimation* anim = scene->mAnimations[i];
		LOG("Importing animation [%s] -----------------", anim->mName.C_Str());
		App->resources->ImportBuffer(anim, 0, Resource::animation, full_path);
	}
}
