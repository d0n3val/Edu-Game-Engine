#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Globals.h"
#include "Module.h"
#include "Resource.h"
#include <map>
#include <vector>

class Resource;
class LoaderBone;
class LoaderAnimation;

class ModuleResources : public Module
{
public:

	ModuleResources(bool start_enabled = true);

	// Destructor
	~ModuleResources();

	bool Init(Config* config) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;

	void SaveResources() const;
	void LoadResources();

	Resource::Type TypeFromExtension(const char* extension) const;
	UID Find(const char* file_in_assets) const;
	UID ImportFileOutsideVFM(const char* full_path);
	UID ImportFile(const char* new_file_in_assets);
	UID ImportBuffer(const void* buffer, uint size, Resource::Type type, const char* source_file = nullptr);
	UID GenerateNewUID();
	const Resource* Get(UID uid) const;
	Resource * Get(UID uid);
	Resource* CreateNewResource(Resource::Type type, UID force_uid = 0);
	//TODO: const Resource* Attach(GameObject* gameobject, UID uid));
	void GatherResourceType(std::vector<const Resource*>& resources, Resource::Type type) const;

	const LoaderBone* GetBoneLoader() const;
	const LoaderAnimation* GetAnimationLoader() const;
	

private:

	void LoadUID();
	void SaveUID() const;

private:
	std::string asset_folder;
	UID last_uid = 1;
	std::map<UID, Resource*> resources;
	LoaderBone* bone_loader = nullptr;
	LoaderAnimation* anim_loader = nullptr;
};

#endif // __MODULERESOURCES_H__