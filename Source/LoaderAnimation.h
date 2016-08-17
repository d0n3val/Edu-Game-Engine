#ifndef __LOADER_ANIMATION_H__
#define __LOADER_ANIMATION_H__

#include "Globals.h"
#include "Module.h"
#include <string>

class ResourceAnimation;
struct aiAnimation;

class LoaderAnimation
{
public:
	LoaderAnimation();
	~LoaderAnimation();

	bool Load(ResourceAnimation* resource) const;
	bool Import(const aiAnimation* bone, UID mesh, std::string& output) const;

private:
	bool Save(const ResourceAnimation& bone, std::string& output) const;
};

#endif // __LOADER_ANIMATION_H__