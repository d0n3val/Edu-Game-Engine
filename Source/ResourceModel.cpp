#include "ResourceModel.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"

#include "Assimp/include/scene.h"

#include "utils/SimpleBinStream.h"
#include "HashString.h"

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
        uint size = App->fs->Load(LIBRARY_MESH_FOLDER, GetExportedFile(), &buffer);

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
            read_stream >> node.mesh;
            read_stream >> node.material;

            nodes.push_back(node);
        }

        for(uint i=0; i< nodes.size(); ++i)
        {
            if(nodes[i].mesh != 0)
            {
                App->resources->Get(nodes[i].mesh)->LoadToMemory();
            }

            if(nodes[i].material != 0)
            {
                App->resources->Get(nodes[i].material)->LoadToMemory();
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
        if(nodes[i].mesh != 0)
        {
            App->resources->Get(nodes[i].mesh)->Release();
        }

        if(nodes[i].material != 0)
        {
            App->resources->Get(nodes[i].material)->Release();
        }
    }

    nodes.clear();
}

bool ResourceModel::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    write_stream << uint(nodes.size());

    for(uint i=0; i< nodes.size(); ++i)
    {
        write_stream << nodes[i].name;
        write_stream << nodes[i].transform;
        write_stream << nodes[i].parent;
        write_stream << nodes[i].mesh;
        write_stream << nodes[i].material;
    }

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MESH_FOLDER, "model", "edumodel");
}

UID ResourceModel::Import(const aiScene* model, const std::vector<UID>& meshes, const std::vector<UID>& materials, const char* source_file)
{
    ResourceModel* m = static_cast<ResourceModel*>(App->resources->CreateNewResource(Resource::model));

    m->GenerateNodes(model, model->mRootNode, 0, meshes, materials);

    std::string output;

    bool save_ok = m->Save(output);
    m->nodes.clear();

    if(save_ok)
    {
		if (source_file != nullptr) 
        {
			m->file = source_file;
			App->fs->NormalizePath(m->file);
		}

		std::string file;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file);
		m->exported_file = file;

		LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());

        return m->uid;
    }

    LOG("Importing of model aiScene [%s] FAILED", source_file);

	return 0;
}

void ResourceModel::GenerateNodes(const aiScene* model, const aiNode* node, uint parent, const std::vector<UID>& meshes, const std::vector<UID>& materials)
{
    uint index = nodes.size();

	Node dst;

    dst.transform = reinterpret_cast<const float4x4&>(node->mTransformation);
    dst.name      = node->mName.C_Str();
    dst.parent    = parent;

    nodes.push_back(dst);

    for(unsigned i=0; i < node->mNumChildren; ++i)
    {
        GenerateNodes(model, node->mChildren[i], index, meshes, materials);
    }

    if(node->mNumMeshes == 1)
    {
        uint mesh_index = node->mMeshes[0];

        nodes[index].mesh     = meshes[mesh_index];
        nodes[index].material = materials[model->mMeshes[mesh_index]->mMaterialIndex];
    }
    else 
    {
        for(uint i=0; i< node->mNumMeshes; ++i)
        {
            Node mesh;

            uint mesh_index = node->mMeshes[i];

            mesh.parent   = index;
            mesh.name     = model->mMeshes[mesh_index]->mName.C_Str();
            mesh.mesh     = meshes[mesh_index];
            mesh.material = materials[model->mMeshes[mesh_index]->mMaterialIndex];

            nodes.push_back(mesh);
        }
    }
}


