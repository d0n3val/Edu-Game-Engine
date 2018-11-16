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
        model,
        material,
		texture,
		mesh,
		audio,
		scene,
		bone,
		animation,
		unknown
	};

public:
	Resource(UID uid, Resource::Type type);
	virtual ~Resource();

	Resource::Type  GetType() const;
	const char*     GetTypeStr() const;
	UID             GetUID() const;
	const char*     GetFile() const;
	const char*     GetExportedFile() const;

	bool            IsLoadedToMemory() const;
	bool            LoadToMemory();
    void            Release();

	uint            CountReferences() const;

	virtual void    Save(Config& config) const;
	virtual void    Load(const Config& config);

protected:

	virtual bool    LoadInMemory() = 0;
    virtual void    ReleaseFromMemory() = 0;

protected:

	UID uid = 0;
	std::string file;
	std::string exported_file;

	Type type = unknown;
	uint loaded = 0;
};

#endif // __Resource_H__
