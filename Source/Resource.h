#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// Base class for all possible game resources

#include "Globals.h"
#include <string>

class Config;

class Resource
{
	friend class ModuleResources;
public:
	enum Type {
		texture,
		mesh,
		audio,
		scene,
		unknown
	};

public:
	Resource(UID uid, Resource::Type type);
	virtual ~Resource();

	Resource::Type GetType() const;
	const char* GetTypeStr() const;
	UID GetUID() const;
	const char* GetFile() const;
	const char* GetExportedFile() const;

	virtual void Save(Config& config) const;
	virtual void Load(const Config& config);

private:

	UID uid = 0;
	std::string file;
	std::string exported_file;

	Type type = unknown;

};

#endif // __Resource_H__