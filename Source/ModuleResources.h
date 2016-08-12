#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Globals.h"
#include "Module.h"
#include <map>


struct Resource
{
	UID uid = 0;
	std::string file;
	std::string exported_file;

	enum Type{
		image,
		mesh,
		music,
		effect,
		scene,
		unknown
	} type = unknown;

	Resource(UID uid, Resource::Type type) : uid(uid), type(type)
	{}
};
  
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
	UID ImportFile(const char* full_path, const char* destination);
	UID ImportFile(const char* new_file_in_assets);
	UID GenerateNewUID();
	const Resource* Get(UID uid) const;

private:

	void LoadUID();
	void SaveUID() const;

private:
	std::string asset_folder;
	UID last_uid = 1;
	std::map<UID, Resource*> resources;
};

#endif // __MODULERESOURCES_H__