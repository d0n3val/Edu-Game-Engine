#include "Globals.h"
#include "Application.h"
#include "ModuleSceneLoader.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentBone.h"
#include "Config.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
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

	const aiScene* scene = aiImportFile(full_path.data,  flags);

	if (scene)
	{
		GenerateMaterials(scene, path);
		node = GenerateGameObjects(scene);

		if(node)
		{
			LinkNode(node, &root);
		}

		unsigned first_material = materials.size();

		GenerateMeshes(scene, first_material);

		aiReleaseImport(scene);
	}

	return node;
}


GameObject* ModuleSceneLoader::GenerateGameObjects(const aiScene* scene)
{
	GameObject* node = new GameObject;
	GenerateGameObjectsRecursive(scene->mRootNode, node);

	return node;
}

void ModuleSceneLoader::GenerateGameObjectsRecursive(const aiNode* src, Node* dst)
{
	dst->name = HashString(src->mName.C_Str());

    aiQuaternion quat;
	src->mTransformation.Decompose(*reinterpret_cast<aiVector3D*>(&dst->scale), quat, *reinterpret_cast<aiVector3D*>(&dst->position));

    dst->rotation = math::float4(quat.x, quat.y, quat.z, quat.w);

    // \todo: add mesh components

	dst->childs.resize(src->mNumChildren);

	for(unsigned i=0; i < src->mNumChildren; ++i)
	{
		dst->childs[i] = new GameObject;
		dst->childs[i]->parent = dst;

		GenerateNodesRecursive(src->mChildren[i], dst->childs[i]);
	}
}

void ModuleSceneLoader::GenerateMaterials(const aiScene* scene, const char* file, dtl::vector<UID>& materials)
{
	materials.reserve(scene->mNumMaterials);

	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
        materials.push_back(App->resources->ImportBuffer(scene->mMaterials[i], 0, Resource::material, file));
	}
}

void ModuleSceneLoader::GenerateMeshes(const aiScene* scene, const std::vector<UID>& materials, std::vector<UID>& meshes)
{
	meshes.resize(scene->mNumMeshes);

	for(unsigned i=0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* orig = scene->mMeshes[i];
		Mesh* dst = meshes[i] = new Mesh;

        dst->name = orig->mName.length > 0 ? HashString(orig->mName.C_Str()) : HashString();
		dst->material = orig->mMaterialIndex+first_material;

        GenerateAttribInfo(dst, orig);
        GenerateCPUBuffers(dst, orig);

        if((dst->attribs & ATTRIB_BONES) != 0)
        {
            GenerateBoneData(dst, orig);
        }

        GenerateVBO(dst, false);
        GenerateVAO(dst);
	}
}

void Scene::GenerateAttribInfo(Mesh* dst, const aiMesh* orig)
{
    dst->vertex_size     = sizeof(math::float3);
    dst->texcoord_offset = 0;
    dst->normal_offset   = 0;

    if(orig->HasNormals())
    {
        dst->attribs |= ATTRIB_NORMALS;
        dst->normal_offset = dst->vertex_size*orig->mNumVertices;
        dst->vertex_size += sizeof(math::float3);
    }

    if(orig->HasTextureCoords(0))
    {
        dst->attribs |= ATTRIB_TEX_COORDS_0;
        dst->texcoord_offset = dst->vertex_size*orig->mNumVertices;
        dst->vertex_size += sizeof(aiVector2D);
    }

    if(orig->HasTangentsAndBitangents())
    {
        dst->attribs |= ATTRIB_TANGENTS;
        dst->tangent_offset = dst->vertex_size*orig->mNumVertices;
        dst->vertex_size += sizeof(math::float3);
    }

    if(orig->HasBones())
    {
        dst->attribs |= ATTRIB_BONES;
        dst->bone_idx_offset = dst->vertex_size*orig->mNumVertices;
        dst->vertex_size += sizeof(unsigned)*4;
        dst->bone_weight_offset = dst->vertex_size*orig->mNumVertices;
        dst->vertex_size += sizeof(float)*4;
    }
}

void Scene::GenerateCPUBuffers(Mesh* dst, const aiMesh* orig)
{
    dst->num_vertices = orig->mNumVertices;

    dst->src_vertices = new math::float3[orig->mNumVertices];

    math::float3 min(FLT_MAX);
    math::float3 max(-FLT_MAX);

    for(unsigned i=0; i< orig->mNumVertices; ++i)
    {
        dst->src_vertices[i] = *((math::float3*)&orig->mVertices[i]);

        for(unsigned j=0; j<3; ++j)
        {
            min[j] = std::min(min[j], dst->src_vertices[i][j]);
            max[j] = std::max(max[j], dst->src_vertices[i][j]);
        }
    }

    dst->oobb.center    = (min+max)*0.5f;
    dst->oobb.half_size = (max-min)*0.5f;

    if(orig->HasTextureCoords(0))
    {
        dst->src_texcoord0 = new math::float2[orig->mNumVertices];

        for(unsigned i=0; i < orig->mNumVertices; ++i) 
        {
            dst->src_texcoord0[i] = math::float2(orig->mTextureCoords[0][i].x, orig->mTextureCoords[0][i].y);
        }
    }

    if(orig->HasNormals())
    {
        dst->src_normals = new math::float3[orig->mNumVertices];
        memcpy(dst->src_normals, orig->mNormals, sizeof(math::float3)*orig->mNumVertices);
    }

    dst->src_indices = new unsigned[orig->mNumFaces*3];
    dst->num_indices = orig->mNumFaces*3;

    for(unsigned j=0; j < orig->mNumFaces; ++j)
    {
        const aiFace& face = orig->mFaces[j];

        assert(face.mNumIndices == 3);

        dst->src_indices[j * 3] = face.mIndices[0];
        dst->src_indices[j * 3 + 1] = face.mIndices[1];
        dst->src_indices[j * 3 + 2] = face.mIndices[2];
    }

    if(orig->HasTangentsAndBitangents())
    {
        // uncomment iif copy form assimp
        dst->src_tangents = new math::float3[orig->mNumVertices];
        memcpy(dst->src_tangents, orig->mTangents, sizeof(math::float3)*orig->mNumVertices);
        //GenerateTangentSpace(dst);
    }
}

void Scene::GenerateVBO(Mesh* dst, bool dynamic)
{
    glGenBuffers(1, &dst->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, dst->vbo);

    glBufferData(GL_ARRAY_BUFFER, dst->vertex_size*dst->num_vertices, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(math::float3)*dst->num_vertices, dst->src_vertices);

    if((dst->attribs & ATTRIB_TEX_COORDS_0) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, dst->texcoord_offset, sizeof(aiVector2D)*dst->num_vertices, dst->src_texcoord0);
    }

    if((dst->attribs & ATTRIB_NORMALS) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, dst->normal_offset, sizeof(math::float3)*dst->num_vertices, dst->src_normals);
    }

    if((dst->attribs & ATTRIB_TANGENTS) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, dst->tangent_offset, sizeof(math::float3)*dst->num_vertices, dst->src_tangents);
    }

    if((dst->attribs & ATTRIB_BONES) != 0)
    {
        unsigned* bone_indices = (unsigned*)glMapBufferRange(GL_ARRAY_BUFFER, dst->bone_idx_offset, 
                                                             (sizeof(unsigned)*4+sizeof(float)*4)*dst->num_vertices, 
                                                             GL_MAP_WRITE_BIT);
        float* bone_weights    = (float*)(bone_indices+dst->num_vertices*4);

        for(unsigned i=0; i < dst->num_vertices*4; ++i) 
        {
            bone_indices[i] = 0;
            bone_weights[i] = 0.0f;
        }

        for(unsigned i=0; i< dst->num_bones; ++i)
        {
            const Bone& bone = dst->bones[i];

            for(unsigned j=0; j < bone.num_weights; ++j)
            {
                unsigned index = bone.weights[j].vertex;
                float weight   = bone.weights[j].weight;

                unsigned* bone_idx = &bone_indices[index*4];
                float* bone_weight = &bone_weights[index*4];

                assert(bone_weight[3] == 0.0f);
                for(unsigned l=0; l < 4; ++l)
                {
                    if(bone_weight[l] == 0.0f)
                    {
                        bone_idx[l] = i;
                        bone_weight[l] = weight;

                        break;
                    }
                }
            }
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &dst->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dst->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, dst->num_indices*sizeof(unsigned), dst->src_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Scene::GenerateBoneData(Mesh* dst, const aiMesh* orig)
{
    assert((dst->attribs & ATTRIB_BONES) != 0);

    dst->bones      = new Bone[orig->mNumBones];
    dst->num_bones  = orig->mNumBones;

    dst->palette    = new math::float4x4[dst->num_bones];
    dst->node_cache = new Node*[dst->num_bones];

    for(unsigned i=0; i< orig->mNumBones; ++i)
    {
        const aiBone* bone   = orig->mBones[i];
        Bone& dst_bone       = dst->bones[i];

        dst_bone.name 	     = HashString(bone->mName.C_Str());
        dst_bone.weights     = new Weight[bone->mNumWeights];
        dst_bone.num_weights = bone->mNumWeights;
        dst_bone.bind 	     = math::float4x4(math::float4(bone->mOffsetMatrix.a1, bone->mOffsetMatrix.b1, bone->mOffsetMatrix.c1, bone->mOffsetMatrix.d1),
											  math::float4(bone->mOffsetMatrix.a2, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.d2),
                                              math::float4(bone->mOffsetMatrix.a3, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.d3),
                                              math::float4(bone->mOffsetMatrix.a4, bone->mOffsetMatrix.b4, bone->mOffsetMatrix.c4, bone->mOffsetMatrix.d4));

        for(unsigned j=0; j < bone->mNumWeights; ++j)
        {
            dst_bone.weights[j].vertex = bone->mWeights[j].mVertexId;
            dst_bone.weights[j].weight = bone->mWeights[j].mWeight;
        }

        dst->node_cache[i] = nullptr;
    }

}

void Scene::GenerateVAO(Mesh* dst)
{
    glGenVertexArrays(1, &dst->vao);
    glBindVertexArray(dst->vao);

	glBindBuffer(GL_ARRAY_BUFFER, dst->vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)0);

	if ((dst->attribs & ATTRIB_NORMALS) != 0)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)(dst->normal_offset));
	}

	if ((dst->attribs & ATTRIB_TEX_COORDS_0) != 0)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(aiVector2D), (void*)(dst->texcoord_offset));
	}

	if((dst->attribs & ATTRIB_BONES) != 0)
	{
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(unsigned)*4, (void*)dst->bone_idx_offset);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)dst->bone_weight_offset);
	}

    if((dst->attribs & ATTRIB_TANGENTS) != 0)
    {
		glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(math::float3), (void*)(dst->tangent_offset));
    }

    glBindVertexArray(0);
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
