#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// Base class for all possible game resources

#include "Globals.h"
#include <string>

class Resource
{
	friend class ModuleResources;
public:
	enum Type {
		texture,
		mesh,
		music,
		effect,
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

private:

	UID uid = 0;
	std::string file;
	std::string exported_file;

	Type type = unknown;

};

#endif // __Resource_H__