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
	return false;
}

void ResourceModel::ReleaseFromMemory() 
{
}

bool ResourceModel::Save(std::string& output) const
{
	return false;
}

UID ResourceModel::Import(const aiScene* model, UID material, const char* source_file)
{
	return 0;
}
