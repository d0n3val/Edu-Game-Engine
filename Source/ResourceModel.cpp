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

				if (renderer.mesh != 0)
				{
					node.renderers.push_back(renderer);
				}
            }

			nodes.push_back(std::move(node));
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

        return App->fs->Save(full_path, &data[0], data.size()) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MODEL_FOLDER, "model", "edumodel"))
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

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MODEL_FOLDER, "model", "edumodel");
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
        }
    }
}


bool ResourceModel::Import(const char* full_path, float scale, std::string& output)
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

    parent = nodes.size();

    nodes.push_back(std::move(dst));

    for(unsigned i=0; i < node->mNumChildren; ++i)
    {
        GenerateNodes(model, node->mChildren[i], parent, meshes, materials, scale);
    }
}


