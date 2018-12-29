#include "Resource.h"
#include "Config.h"

// ---------------------------------------------------------
Resource::Resource(UID uid, Resource::Type type) : uid(uid), type(type)
{}

// ---------------------------------------------------------
Resource::~Resource()
{}
Resource::Type Resource::GetType() const
{
	return type;
}
// ---------------------------------------------------------
const char * Resource::GetTypeStr() const
{
    return GetTypeStr(type);
}

// ---------------------------------------------------------
const char* Resource::GetTypeStr(Type type) 
{
	static_assert(Resource::Type::unknown == 6, "String list needs update");

	static const char* types[] = { "Model", "Material", "Texture", "Mesh", "Audio", "Animation", "Unknown" };

	return types[type];
}

// ---------------------------------------------------------
UID Resource::GetUID() const
{
	return uid;
}
// ---------------------------------------------------------
const char * Resource::GetFile() const
{
	return file.c_str();
}
// ---------------------------------------------------------
const char * Resource::GetExportedFile() const
{
	return exported_file.c_str();
}

// ---------------------------------------------------------
bool Resource::IsLoadedToMemory() const
{
	return loaded > 0;
}

// ---------------------------------------------------------
bool Resource::LoadToMemory()
{
	if (loaded > 0)
		loaded++;
	else
		loaded = LoadInMemory() ? 1 : 0;
	
	return loaded > 0;
}

// ---------------------------------------------------------
void Resource::Release()
{
    if(--loaded == 0)
    {
        ReleaseFromMemory();
    }
}

// ---------------------------------------------------------
uint Resource::CountReferences() const
{
	return loaded;
}

// ---------------------------------------------------------
void Resource::Save(Config & config) const
{
	config.AddUID("UID", uid);
	config.AddInt("Type", type);
	config.AddString("File", file.c_str());
	config.AddString("Exported", exported_file.c_str());
}

// ---------------------------------------------------------
void Resource::Load(const Config & config)
{
	uid = config.GetUID("UID", 0);
	type = (Type) config.GetInt("Type", unknown);
	file = config.GetString("File", "???");
	exported_file = config.GetString("Exported", "???");
} 

