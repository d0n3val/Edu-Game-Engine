#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModuleSceneLoader.h"
#include "Event.h"
#include <string>

#define LAST_UID_FILE "LAST_UID"

using namespace std;

ModuleResources::ModuleResources(bool start_enabled) : Module("Resource Manager", start_enabled), asset_folder(ASSETS_FOLDER)
{
	// Exception: we are going to acces file system to obtain the last used unique id
	// so others modules can use resources starting from Init()
}

// Destructor
ModuleResources::~ModuleResources()
{
}

// Called before render is available
bool ModuleResources::Init(Config* config)
{
	LOG("Loading Resource Manager");
	bool ret = true;

	LoadUID();

	return ret;
}

// Called before quitting
bool ModuleResources::CleanUp()
{
	LOG("Unloading Resource Manager");

	return true;
}

void ModuleResources::ReceiveEvent(const Event& event)
{
	switch (event.type)
	{
		case Event::file_dropped:
			LOG("File dropped: %s", event.string.ptr);
			ImportFile(event.string.ptr);
		break;
	}
}

Resource::Type ModuleResources::TypeFromExtension(const char * extension) const
{
	Resource::Type ret = Resource::unknown;

	if (extension != nullptr)
	{
		if (_stricmp(extension, "wav") == 0)
			ret = Resource::effect;
		else if (_stricmp(extension, "ogg") == 0)
			ret = Resource::music;
		else if (_stricmp(extension, "dds") == 0)
			ret = Resource::image;
		else if (_stricmp(extension, "png") == 0)
			ret = Resource::image;
		else if (_stricmp(extension, "jpg") == 0)
			ret = Resource::image;
		else if (_stricmp(extension, "tga") == 0)
			ret = Resource::image;
		else if (_stricmp(extension, "fbx") == 0)
			ret = Resource::scene;
		else if (_stricmp(extension, "dae") == 0)
			ret = Resource::scene;
	}

	return ret;
}

UID ModuleResources::ImportFile(const char * full_path, const char * destination)
{
	UID ret = 0;

	string final_path;
	string extension;
	if (destination == nullptr)
	{
		App->fs->SplitFilePath(full_path, nullptr, &final_path, &extension);
		final_path = asset_folder + final_path;
	}
	else
		final_path = asset_folder + destination;

	if(App->fs->Copy(full_path, final_path.c_str()) == true)
		ret = ImportFile(destination);

	return ret;
}

UID ModuleResources::ImportFile(const char * new_file_in_assets)
{
	string extension;
	App->fs->SplitFilePath(new_file_in_assets, nullptr, nullptr, &extension);

	Resource* res = new Resource(GenerateNewUID(), TypeFromExtension(extension.c_str()));
	resources[res->uid] = res;
	res->file = new_file_in_assets;

	switch (res->type)
	{
		case Resource::image:
			res->exported_file = App->tex->Import(new_file_in_assets, "");
		break;

		default:
		break;
	}

	return res->uid;
}

UID ModuleResources::GenerateNewUID()
{
	++last_uid;
	SaveUID();
	return last_uid;
}

const Resource * ModuleResources::Get(UID uid) const
{			   
	if(resources.find(uid) != resources.end())
		return resources.at(uid);
	return nullptr;
}

void ModuleResources::LoadUID()
{
	string file(SETTINGS_FOLDER);
	file += LAST_UID_FILE;

	char *buf = nullptr;
	uint size = App->fs->Load(file.c_str(), &buf);

	if (size == sizeof(last_uid))
		last_uid = *((UID*)buf);
	else
		LOG("WARNING! Cannot read resource UID from file [%s]", file.c_str());

	RELEASE(buf);
}

void ModuleResources::SaveUID() const
{
	string file(SETTINGS_FOLDER);
	file += LAST_UID_FILE;

	uint size = App->fs->Save(file.c_str(), (const char*) &last_uid, sizeof(last_uid));

	if (size != sizeof(last_uid))
		LOG("WARNING! Cannot write resource UID into file [%s]", file.c_str());
}
