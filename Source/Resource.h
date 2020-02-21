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
	enum Type 
    {
        model = 0,
        material,
		texture,
		mesh,
		audio,
		animation,
        state_machine,
		unknown
	};

public:
	Resource(UID uid, Resource::Type type);
	virtual ~Resource();

	Resource::Type      GetType() const;
	const char*         GetTypeStr() const;
	UID                 GetUID() const;
	const char*         GetFile() const;
	const char*         GetExportedFile() const;
    const char*         GetSourceName() const { return user_name.c_str(); }

	bool                IsLoadedToMemory() const;
	bool                LoadToMemory();
    void                Release();

	uint                CountReferences() const;

    //virtual bool        Save() = 0;
	virtual void        Save(Config& config) const;
	virtual void        Load(const Config& config);

    static const char*  GetTypeStr(Type type);

    void                SetName(const char* n) { user_name = n; }

protected:

	virtual bool    LoadInMemory() = 0;
    virtual void    ReleaseFromMemory() = 0;

protected:

	UID uid = 0;
	std::string file;
	std::string exported_file;
    std::string user_name;

	Type type = unknown;
	uint loaded = 0;
};

#endif // __Resource_H__
