#ifndef __LOADER_ANIMATION_H__
#define __LOADER_ANIMATION_H__

#include "Globals.h"
#include "Module.h"
#include "ResourceAnimation.h"
#include <string>

struct aiAnimation;
struct aiNodeAnim;

class LoaderAnimation
{
public:
	LoaderAnimation();
	~LoaderAnimation();

	bool Load(ResourceAnimation* resource) const;
	bool Import(const aiAnimation* bone, UID mesh, std::string& output) const;

private:
	bool Save(const ResourceAnimation& bone, std::string& output) const;
	void ImportBoneTransform(const aiNodeAnim* node, ResourceAnimation::bone_transform& bone) const;
};

#endif // __LOADER_ANIMATION_H__