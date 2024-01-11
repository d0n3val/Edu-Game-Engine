#include "Globals.h"

#define TINYGLTF_IMPLEMENTATION
#include "gltf.h"

#include "ResourceModel.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"

#include "Assimp/scene.h"
#include "Assimp/cimport.h"
#include "Assimp/postprocess.h"

#include "ResourceMaterial.h"
#include "ResourceMesh.h"

#include "utils/SimpleBinStream.h"
#include "HashString.h"

#include "Leaks.h"

#pragma comment (lib, "Assimp/lib/assimp-vc142-mt.lib")


ResourceModel::ResourceModel(UID id) : Resource(id, Resource::Type::model)
{
}

ResourceModel::~ResourceModel()
{
}

void ResourceModel::Save(Config& config) const 
{
	Resource::Save(config);
}

void ResourceModel::Load(const Config& config) 
{
	Resource::Load(config);
}

bool ResourceModel::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;
        uint size = App->fs->Load(LIBRARY_MODEL_FOLDER, GetExportedFile(), &buffer);

        simple::mem_istream<std::true_type> read_stream(buffer, size);

        uint node_size = 0;

        read_stream >> node_size;

        nodes.reserve(node_size);

        for(uint i=0; i < node_size; ++i)
        {
            Node node;

            read_stream >> node.name;

            for(uint i=0; i< 16; ++i)
            {
                read_stream >> reinterpret_cast<float*>(&node.transform)[i];
            }

            read_stream >> node.parent;

            uint renderers_size = 0;
            read_stream >> renderers_size;
			
            node.renderers.reserve(renderers_size);

            for(uint j=0; j < renderers_size; ++j)
            {
                MeshRenderer renderer;

                read_stream >> renderer.mesh;
                read_stream >> renderer.material;
                read_stream >> renderer.skin;

				if (renderer.mesh != 0)
				{
					node.renderers.push_back(renderer);
				}
            }

			nodes.push_back(std::move(node));
        }

        uint skinSize = 0;
        read_stream >> skinSize;

        nodes.reserve(skinSize);

        for (uint i = 0; i < skinSize; ++i)
        {
            Skin skin;
            read_stream >> skin.rootNode;

            uint boneSize = 0;
            read_stream >> boneSize;

            skin.bones.reserve(boneSize);
            for (uint j = 0; j < boneSize; ++j)
            {
                SkinBone bone;

                read_stream >> bone.bind;
                read_stream >> bone.nodeIdx;

                skin.bones.push_back(bone);
            }

            skins.push_back(skin);
        }

        for(uint i=0; i< nodes.size(); ++i)
        {
            for(uint j=0; j < nodes[i].renderers.size(); ++j)
            {
                if(nodes[i].renderers[j].mesh != 0)
                {
                    Resource* res = App->resources->Get(nodes[i].renderers[j].mesh);
                    if (res)
                    {
                        res->LoadToMemory();
                    }
                }

                if(nodes[i].renderers[j].material != 0)
                {
                    Resource* res = App->resources->Get(nodes[i].renderers[j].material);
                    if (res)
                    {
                        res->LoadToMemory();
                    }
                }
            }
        }

		return true;
    }

	return false;
}

void ResourceModel::ReleaseFromMemory() 
{
    for(uint i=0; i< nodes.size(); ++i)
    {
        for(uint j=0; j < nodes[i].renderers.size(); ++j)
        {
            if(nodes[i].renderers[j].mesh != 0)
            {
                App->resources->Get(nodes[i].renderers[j].mesh)->Release();
            }

            if(nodes[i].renderers[j].material != 0)
            {
                App->resources->Get(nodes[i].renderers[j].material)->Release();
            }
        }
    }

    nodes.clear();
}

// ---------------------------------------------------------
bool ResourceModel::Save() 
{
    simple::mem_ostream<std::true_type> write_stream;

    LoadToMemory();
    SaveToStream(write_stream);
    Release();

    const std::vector<char>& data = write_stream.get_internal_vec();

    if(exported_file.length() > 0)
    {
		char full_path[250];

		sprintf_s(full_path, 250, "%s%s", LIBRARY_MODEL_FOLDER, exported_file.c_str());

        return App->fs->Save(full_path, &data[0], int(data.size())) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], int(data.size()), LIBRARY_MODEL_FOLDER, "model", "edumodel"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }


	return false;
}

bool ResourceModel::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], int(data.size()), LIBRARY_MODEL_FOLDER, "model", "edumodel");
}

void ResourceModel::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << uint(nodes.size());

    for(uint i=0; i< nodes.size(); ++i)
    {
        write_stream << nodes[i].name;
        write_stream << nodes[i].transform;
        write_stream << nodes[i].parent;

        write_stream << uint(nodes[i].renderers.size());

        for(uint j=0; j< nodes[i].renderers.size(); ++j)
        {
            write_stream << nodes[i].renderers[j].mesh;
            write_stream << nodes[i].renderers[j].material;
            write_stream << nodes[i].renderers[j].skin;
        }
    }

    write_stream << uint(skins.size());

    for (const Skin& skin : skins)
    {
        write_stream << skin.rootNode;
        write_stream << uint(skin.bones.size());

        for (const SkinBone& bone : skin.bones)
        {
            write_stream << bone.bind;
            write_stream << bone.nodeIdx;
        }
    }

}

void ResourceModel::GenerateMeshes(const tinygltf::Model& model, const char* full_path, const std::vector<UID>& materials, 
                                   std::multimap<uint, MeshRenderer>& meshes, float scale)
{
    for (size_t i=0, count = model.meshes.size(); i< count; ++i)
    {
        const tinygltf::Mesh& srcMesh = model.meshes[i];

        for (const auto& primitive : srcMesh.primitives)
        {
            UID material = primitive.material >= 0 ? materials[primitive.material] : 0;

            MeshRenderer renderer = { ResourceMesh::Import(model, srcMesh, primitive, full_path, scale), material };
            
            meshes.insert({ uint(i), renderer});
        }
    }
}

void ResourceModel::GenerateMaterials(const tinygltf::Model& model, const char* file, std::vector<UID>& materials)
{
    materials.reserve(model.materials.size());

    for (const auto& srcMaterial : model.materials)
    {
        materials.push_back(ResourceMaterial::Import(model, srcMaterial, file));
    }
}

void ResourceModel::GenerateSkins(const tinygltf::Model& model, const std::vector<int>& nodeMapping)
{    
    skins.resize(model.skins.size());

    int skinIdx = 0;
    for (const tinygltf::Skin& srcSkin : model.skins)
    {
        Skin& skin = skins[skinIdx++];

        skin.rootNode = srcSkin.skeleton >= 0 ? nodeMapping[srcSkin.skeleton] : -1;

        skin.bones.resize(srcSkin.joints.size());

        if (srcSkin.inverseBindMatrices >= 0)
        {
            const tinygltf::Accessor& bindAcc = model.accessors[srcSkin.inverseBindMatrices];
            const tinygltf::BufferView& bindView = model.bufferViews[bindAcc.bufferView];
            const uint8_t* bindPtr = reinterpret_cast<const uint8_t*>(&(model.buffers[bindView.buffer].data[bindAcc.byteOffset + bindView.byteOffset]));
            size_t bindStride = bindView.byteStride == 0 ? sizeof(float4x4) : bindView.byteStride;

            SDL_assert(bindAcc.count == srcSkin.joints.size());

            for (int i = 0; i < bindAcc.count; ++i)
            {
                skin.bones[i].bind = *reinterpret_cast<const float4x4*>(bindPtr);
                skin.bones[i].bind.Transpose();

                bindPtr += bindStride;
            }
        }

        int jointIdx = 0;
        for (int jointId : srcSkin.joints)
        {
            if (jointId >= 0) skin.bones[jointIdx++].nodeIdx = nodeMapping[jointId];
            else skin.bones[jointIdx++].nodeIdx = -1;
        }
    }
}

void ResourceModel::GenerateNodes(const tinygltf::Model& model, int nodeIndex, int parentIndex, const std::multimap<uint, MeshRenderer>& meshes, 
                                  const std::vector<UID>& materials, std::vector<int>& nodeMapping, float scale)
{
    const tinygltf::Node& node = model.nodes[nodeIndex];

    float4x4 local = float4x4::identity;

    if (node.matrix.size() == 16)
    {
        float* ptr = reinterpret_cast<float*>(&local);  
        for(uint i=0; i< 16; ++i) ptr[i] = float(node.matrix[i]);

        local.Transpose();
    }
    else
    {
        float3 translation = float3::zero;
        float3 scale = float3::one;
        Quat rotation = Quat::identity;

        if (node.translation.size() == 3) translation = float3(float(node.translation[0]), float(node.translation[1]), float(node.translation[2])) * scale;
        if (node.rotation.size() == 4) rotation = Quat(float(node.rotation[0]), float(node.rotation[1]), float(node.rotation[2]), float(node.rotation[3]));
        if (node.scale.size() == 3) scale = float3(float(node.scale[0]), float(node.scale[1]), float(node.scale[2]));

        local = float4x4::FromTRS(translation, rotation, scale);
    }

    Node dst;
    dst.transform = local;
    dst.name = node.name;
    dst.parent = parentIndex;

    if (node.mesh >= 0)
    {        
        for (auto it = meshes.lower_bound(node.mesh); it != meshes.end() && it->first == node.mesh; ++it)
        {
            MeshRenderer renderer = it->second;
            renderer.skin = node.skin;
            dst.renderers.push_back(renderer);
        }
    }

    parentIndex = uint(nodes.size());

    nodeMapping[nodeIndex] = nodes.size();
    nodes.push_back(std::move(dst));

    for (int childIndex : node.children)
    {
        GenerateNodes(model, childIndex, parentIndex, meshes, materials, nodeMapping, scale);
    }
}

bool ResourceModel::ImportGLTF(const char* full_path, float scale, std::string& output)
{
    tinygltf::TinyGLTF gltfContext;
    tinygltf::Model model;
    std::string error, warning;

    if (gltfContext.LoadASCIIFromFile(&model, &error, &warning, full_path))
    {
        ResourceModel m(0);

        std::vector<UID> materials;
        std::multimap<uint, MeshRenderer> meshes;
        std::vector<int> nodeMapping;
        nodeMapping.resize(model.nodes.size(), -1);

        m.GenerateMaterials(model, full_path, materials);
        m.GenerateMeshes(model, full_path, materials, meshes, scale);
       
        m.nodes.reserve(model.nodes.size());

        for (const tinygltf::Scene& scene : model.scenes)
        {
            for (int root : scene.nodes)
            {
                m.GenerateNodes(model, root, int(m.nodes.size()), meshes, materials, nodeMapping, scale);
            }
        }

        m.GenerateSkins(model, nodeMapping);

        return m.Save(output);

    }

    return false;
}

bool ResourceModel::ImportAssimp(const char* full_path, float scale, std::string& output)
{
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

    aiString assimp_path;
    assimp_path.Append(full_path);

    const aiScene* scene = aiImportFile(assimp_path.data, flags);

    if (scene)
    {
        ResourceModel m(0);

        std::vector<UID> materials, meshes;
        m.GenerateMaterials(scene, full_path, materials);
        m.GenerateMeshes(scene, full_path, meshes, scale);
        m.GenerateNodes(scene, scene->mRootNode, 0, meshes, materials, scale);

        aiReleaseImport(scene);

        return m.Save(output);
    }

    return false;
}

bool ResourceModel::Import(const char* full_path, float scale, std::string& output)
{
    return ImportGLTF(full_path, scale, output) || ImportAssimp(full_path, scale, output);
}

void ResourceModel::GenerateMaterials(const aiScene* scene, const char* file, std::vector<UID>& materials)
{
	materials.reserve(scene->mNumMaterials);

	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
        materials.push_back(ResourceMaterial::Import(scene->mMaterials[i], file));

		assert(materials.back() != 0);
	}
}

void ResourceModel::GenerateMeshes(const aiScene* scene, const char* file, std::vector<UID>& meshes, float scale)
{
	meshes.reserve(scene->mNumMeshes);

	for(unsigned i=0; i < scene->mNumMeshes; ++i)
	{
        meshes.push_back(ResourceMesh::Import(scene->mMeshes[i], file, scale)); 

		assert(meshes.back() != 0);
	}
}

void ResourceModel::GenerateNodes(const aiScene* model, const aiNode* node, uint parent, const std::vector<UID>& meshes, const std::vector<UID>& materials, float scale)
{
    Node dst;
    dst.transform = reinterpret_cast<const float4x4&>(node->mTransformation);
    dst.transform.SetTranslatePart(dst.transform.TranslatePart() * scale);
    dst.name      = node->mName.C_Str();
    dst.parent    = parent;

    for(uint i=0; i< node->mNumMeshes; ++i)
    {
        MeshRenderer renderer;

        uint mesh_index   = node->mMeshes[i];

        renderer.mesh     = meshes[mesh_index];
        renderer.material = materials[model->mMeshes[mesh_index]->mMaterialIndex];

        dst.renderers.push_back(renderer);
    }

    parent = uint(nodes.size());

    nodes.push_back(std::move(dst));

    for(unsigned i=0; i < node->mNumChildren; ++i)
    {
        GenerateNodes(model, node->mChildren[i], parent, meshes, materials, scale);
    }
}


