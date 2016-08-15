#ifndef __LOADER_BONE_H__
#define __LOADER_BONE_H__

#include "Globals.h"
#include "Module.h"

class ResourceBone;
struct aiBone;

class LoaderBone
{
public:
	LoaderBone();
	~LoaderBone();

	bool Load(ResourceBone* resource) const;
	bool Import(const aiBone* bone, UID mesh, std::string& output) const;

private:
	bool Save(const ResourceBone& bone, std::string& output) const;
};

#endif // __LOADER_BONE_H__