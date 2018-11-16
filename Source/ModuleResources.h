#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Globals.h"
#include "Module.h"
#include "Resource.h"
#include <map>
#include <vector>

#define RESERVED_RESOURCES 6 // cube/sphere/cylinde/cone/pyramid primitives + checker texture

class Resource;
class LoaderAnimation;
class ResourceMesh;
class ResourceTexture;

class ModuleResources : public Module
{
public:

	ModuleResources(bool start_enabled = true);

	// Destructor
	~ModuleResources();

	bool Init(Config* config) override;
	bool Start(Config* config) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;

	void SaveResources() const;
	void LoadResources();

	Resource::Type TypeFromExtension(const char* extension) const;
	UID Find(const char* file_in_assets) const;
	UID ImportFileOutsideVFM(const char* full_path);
	UID ImportFile(const char* new_file_in_assets, bool force = false);
	UID ImportBuffer(const void* buffer, uint size, Resource::Type type, const char* source_file = nullptr);
	UID GenerateNewUID();
	const Resource* Get(UID uid) const;
	Resource * Get(UID uid);
	Resource* CreateNewResource(Resource::Type type, UID force_uid = 0);
	//TODO: const Resource* Attach(GameObject* gameobject, UID uid));
	void GatherResourceType(std::vector<const Resource*>& resources, Resource::Type type) const;

	const LoaderAnimation* GetAnimationLoader() const;
	
private:

	void LoadUID();
	void SaveUID() const;

private:
	std::string asset_folder;
	UID last_uid = RESERVED_RESOURCES + 1; // reserve 1 for standard cube mesh
	std::map<UID, Resource*> resources;
	LoaderAnimation* anim_loader = nullptr;

	// Presets
	ResourceMesh* cube = nullptr;
	ResourceMesh* sphere = nullptr;
	ResourceMesh* cone = nullptr;
	ResourceMesh* pyramid = nullptr;
	ResourceMesh* cylinder = nullptr;
	ResourceTexture* checkers = nullptr;
};

#endif // __MODULERESOURCES_H__
