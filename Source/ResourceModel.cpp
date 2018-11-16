#include "ResourceModel.h"


ResourceModel::ResourceModel(UID id) : Resource(uid, Resource::Type::model)
{
}

ResourceModel::~ResourceModel()
{
}

void ResourceModel::Save(Config& config) const 
{
}

void ResourceModel::Load(const Config& config) 
{
}

bool ResourceModel::LoadInMemory()
{
}

void ResourceModel::ReleaseFromMemory() override
{
}

bool ResourceModel::Save(std::string& output) const
{
}

UID ResourceModel::Import(const aiScene* model, UID material, const char* source_file)
{
}
