#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Globals.h"
#include "Module.h"
#include "Resource.h"
#include <map>
#include <vector>

class Resource;

class ModuleResources : public Module
{
public:

	ModuleResources(bool start_enabled = true);

	// Destructor
	~ModuleResources();

	bool Init(Config* config) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;

	Resource::Type TypeFromExtension(const char* extension) const;
	UID Find(const char* file_in_assets) const;
	UID ImportFile(const char* full_path, const char* destination);
	UID ImportFile(const char* new_file_in_assets);
	UID ImportBuffer(const char* buffer, uint size, Resource::Type type);
	UID GenerateNewUID();
	const Resource* Get(UID uid) const;
	Resource* CreateNewResource(Resource::Type type);
	//const Resource* Attach(GameObject* gameobject, UID uid));
	void GatherResourceType(std::vector<const Resource*>& resources, Resource::Type type) const;

private:

	void LoadUID();
	void SaveUID() const;

private:
	std::string asset_folder;
	UID last_uid = 1;
	std::map<UID, Resource*> resources;
};

#endif // __MODULERESOURCES_H__