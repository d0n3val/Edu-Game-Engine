#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "Event.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourceAudio.h"
#include "ResourceModel.h"
#include "ResourceAnimation.h"
#include "ResourceStateMachine.h"
#include "Config.h"
#include <string>

#include "mmgr/mmgr.h"

#define LAST_UID_FILE "LAST_UID"

using namespace std;

ModuleResources::ModuleResources(bool start_enabled) : Module("Resource Manager", start_enabled), asset_folder(ASSETS_FOLDER)
{
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
	LoadResources();

	return ret;
}

bool ModuleResources::Start(Config * config)
{
/* \todo: 
	// Load preset geom shapes in fixed UIDs
	cube = (ResourceMesh*) CreateNewResource(Resource::Type::mesh, 1);
	App->meshes->LoadCube(cube);
	cube->loaded = 1;

	// Load preset geom shapes in fixed UIDs
	sphere = (ResourceMesh*) CreateNewResource(Resource::Type::mesh, 3);
	App->meshes->LoadSphere(sphere);
	sphere->loaded = 1;

	// Load preset geom shapes in fixed UIDs
	cone = (ResourceMesh*) CreateNewResource(Resource::Type::mesh, 4);
	App->meshes->LoadCone(cone);
	cone->loaded = 1;

	// Load preset geom shapes in fixed UIDs
	cylinder = (ResourceMesh*) CreateNewResource(Resource::Type::mesh, 5);
	App->meshes->LoadCylinder(cylinder);
	cylinder->loaded = 1;

	// Load preset geom shapes in fixed UIDs
	pyramid = (ResourceMesh*) CreateNewResource(Resource::Type::mesh, 6);
	App->meshes->LoadPyramid(pyramid);
	pyramid->loaded = 1;

	// Load preset for checkers texture
	checkers = (ResourceTexture*) CreateNewResource(Resource::Type::texture, 2);
	App->tex->LoadCheckers(checkers);
	checkers->loaded = 1;
*/

	// Load preset for checkers texture
	checkers = (ResourceTexture*) CreateNewResource(Resource::Type::texture, 2);
	App->tex->LoadCheckers(checkers);
	checkers->loaded = 1;

	white_fallback = new ResourceTexture(0); 
    black_fallback = new ResourceTexture(0);

    App->tex->LoadFallback(white_fallback, float3(1.0f));
    App->tex->LoadFallback(black_fallback, float3(0.0f));

	return true;
}

// Called before quitting
bool ModuleResources::CleanUp()
{
	LOG("Unloading Resource Manager");

	SaveResources();

	for (map<UID, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
		RELEASE(it->second);

	for (vector<Resource*>::iterator it = removed.begin(); it != removed.end(); ++it)
		RELEASE(*it);

	resources.clear();

	delete white_fallback;
    delete black_fallback;

	return true;
}

void ModuleResources::ReceiveEvent(const Event& event)
{
	switch (event.type)
	{
		case Event::file_dropped:
			LOG("File dropped: %s", event.string.ptr);
			ImportFileOutsideVFM(event.string.ptr);
		break;
	}
}

void ModuleResources::SaveResources() const
{
	bool ret = true;

	Config save;

	// Add header info
	Config desc(save.AddSection("Header"));

	// Serialize GameObjects recursively
	save.AddArray("Resources");

	for (map<UID, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->first > RESERVED_RESOURCES)
		{
			Config resource;
			it->second->Save(resource);

			save.AddArrayEntry(resource);
		}
	}

	// Finally save to file
	char* buf = nullptr;
	uint size = save.Save(&buf, "Resources setup from the EDU Engine");
	App->fs->Save(SETTINGS_FOLDER "resources.json", buf, size);
	RELEASE_ARRAY(buf);
}

void ModuleResources::SaveResourcesTo(const char* path)
{
	bool ret = true;

	// Make sure standard paths exist
	const char* dirs[] = {
		LIBRARY_FOLDER, SETTINGS_FOLDER, 
		LIBRARY_AUDIO_FOLDER, LIBRARY_MESH_FOLDER,
		LIBRARY_MATERIAL_FOLDER, LIBRARY_SCENE_FOLDER, LIBRARY_MODEL_FOLDER, 
		LIBRARY_TEXTURES_FOLDER, LIBRARY_ANIMATION_FOLDER, LIBRARY_STATE_MACHINE_FOLDER,
	};

    char tmp[256];
    char tmp2[256];

	for (uint i = 0; i < sizeof(dirs)/sizeof(const char*); ++i)
	{
		sprintf_s(tmp, 255, "/%s%s", path, dirs[i]);
		if (App->fs->Exists(tmp) == 0)
			App->fs->CreateDirectory(tmp);
	}

	Config save;

	// Add header info
	Config desc(save.AddSection("Header"));

	// Serialize GameObjects recursively
	save.AddArray("Resources");

	for (map<UID, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->first > RESERVED_RESOURCES)
		{
			Config resource;
			it->second->Save(resource);

            sprintf_s(tmp, 255, "%s%s", GetDirByType(it->second->GetType()), it->second->GetExportedFile());
            sprintf_s(tmp2, 255, "/%s%s%s", path, GetDirByType(it->second->GetType()), it->second->GetExportedFile());
            App->fs->Copy(tmp, tmp2);

			save.AddArrayEntry(resource);
		}
	}

	// Finally save to file
	char* buf = nullptr;
	uint size = save.Save(&buf, "Resources setup from the EDU Engine");

    sprintf_s(tmp, 255, "/%s%s%s", path, SETTINGS_FOLDER, "resources.json");
    App->fs->Save(tmp, buf, size);
	RELEASE_ARRAY(buf);
}

const char* ModuleResources::GetDirByType(Resource::Type type) const
{
    static_assert(Resource::Type::unknown == 7, "String list needs update");

	static const char* dirs_by_type[] = {
		LIBRARY_MODEL_FOLDER, LIBRARY_MATERIAL_FOLDER, LIBRARY_TEXTURES_FOLDER, 
        LIBRARY_MESH_FOLDER, LIBRARY_AUDIO_FOLDER, LIBRARY_ANIMATION_FOLDER, 
        LIBRARY_STATE_MACHINE_FOLDER, LIBRARY_SCENE_FOLDER
	};

    return dirs_by_type[type];
}

void ModuleResources::LoadResources()
{
	char* buffer = nullptr;
	uint size = App->fs->Load(SETTINGS_FOLDER "resources.json", &buffer);

	if (buffer != nullptr && size > 0)
	{
		Config config(buffer);

		// Load level description
		Config desc(config.GetSection("Header"));

		int count = config.GetArrayCount("Resources");
		for (int i = 0; i < count; ++i)
		{
			Config resource(config.GetArray("Resources", i));
			Resource::Type type = (Resource::Type) resource.GetInt("Type");
			UID uid = resource.GetUID("UID");

			if (Get(uid) != nullptr)
			{
				LOG("Skipping suplicated resource id %llu", uid);
				continue;
			}

			Resource* res = CreateNewResource(type, uid);
			res->Load(config.GetArray("Resources", i));
		}
		RELEASE_ARRAY(buffer); 
	}
}

Resource::Type ModuleResources::TypeFromExtension(const char * extension) const
{
	Resource::Type ret = Resource::unknown;

	if (extension != nullptr)
	{
		if (_stricmp(extension, "wav") == 0)
			ret = Resource::audio;
		else if (_stricmp(extension, "ogg") == 0)
			ret = Resource::audio;
		else if (_stricmp(extension, "dds") == 0)
			ret = Resource::texture;
		else if (_stricmp(extension, "png") == 0)
			ret = Resource::texture;
		else if (_stricmp(extension, "jpg") == 0)
			ret = Resource::texture;
		else if (_stricmp(extension, "tga") == 0)
			ret = Resource::texture;
		else if (_stricmp(extension, "tif") == 0)
			ret = Resource::texture;
		else if (_stricmp(extension, "fbx") == 0)
			ret = Resource::model;
		else if (_stricmp(extension, "dae") == 0)
			ret = Resource::model;
	}

	return ret;
}

UID ModuleResources::Find(const char * file_in_assets) const
{
	string file(file_in_assets);
	App->fs->NormalizePath(file);

	for (map<UID, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->file.compare(file) == 0)
			return it->first;
	}
	return 0;
}


UID ModuleResources::ImportFileOutsideVFM(const char * full_path)
{
	UID ret = 0;

	string final_path;

	App->fs->SplitFilePath(full_path, nullptr, &final_path);
	final_path = asset_folder + final_path;

	if (App->fs->CopyFromOutsideFS(full_path, final_path.c_str()) == true)
	{
		std::string extension;
		App->fs->SplitFilePath(full_path, nullptr, nullptr, &extension);
		Resource::Type type = TypeFromExtension(extension.c_str());
		ret = ImportFile(final_path.c_str(), type);
	}

	return ret;
}


UID ModuleResources::ImportFile(const char * new_file_in_assets, Resource::Type type, bool force)
{
	UID ret = 0;

	// Check is that file has been already exported
	if (force == true)
	{
		ret = Find(new_file_in_assets);

		if (ret != 0)
			return ret;
	}

	bool import_ok = false;
	string written_file;

	switch (type)
	{
		case Resource::texture:
			import_ok = App->tex->Import(new_file_in_assets, "", written_file, true);
		break;
		case Resource::audio:
			import_ok = App->audio->Import(new_file_in_assets, written_file);
		break;
		case Resource::model:
			import_ok = ResourceModel::Import(new_file_in_assets, written_file);
		break;
		case Resource::animation:
			import_ok = ResourceAnimation::Import(new_file_in_assets, 0, UINT_MAX, written_file);
        break;
	}

	// If export was successfull, create a new resource
	if (import_ok == true)
	{
        std::string user_name;
        App->fs->SplitFilePath(new_file_in_assets, nullptr, &user_name);
        ret = ImportSuccess(type, new_file_in_assets, user_name.c_str(), written_file);
	}
	else
		LOG("Importing of [%s] FAILED", new_file_in_assets);

	return ret;
}

UID ModuleResources::ImportTexture(const char* file_name, bool compressed, bool mipmaps, bool srgb)
{
	UID ret = 0;
    bool import_ok = false;
    string written_file;

    import_ok = App->tex->Import(file_name, "", written_file, compressed);

	// If export was successfull, create a new resource
	if (import_ok == true)
	{
        ret = ImportSuccess(Resource::texture, file_name, "", written_file);
        ResourceTexture* texture = static_cast<ResourceTexture*>(Get(ret));
        texture->EnableMips(mipmaps);
        texture->SetLinear(!srgb);
	}
	else
    {
		LOG("Importing of [%s] FAILED", file_name);
    }

	return ret;
}

UID ModuleResources::ImportAnimation(const char* file_name, uint first, uint last, const char* user_name)
{
	UID ret = 0;
    bool import_ok = false;
    string written_file;

    import_ok = ResourceAnimation::Import(file_name, first, last, written_file);

	// If export was successfull, create a new resource
	if (import_ok == true)
	{
        ret = ImportSuccess(Resource::animation, file_name, user_name, written_file);
	}
	else
    {
		LOG("Importing of [%s] FAILED", file_name);
    }

	return ret;
}

UID ModuleResources::ImportSuccess(Resource::Type type, const char* file_name, const char* user_name, const std::string& output)
{
    Resource* res = CreateNewResource(type);
    res->file = file_name;
    App->fs->NormalizePath(res->file);
    string file;
    App->fs->SplitFilePath(output.c_str(), nullptr, &file);
    res->exported_file = file.c_str();
    LOG("Imported successful from [%s] to [%s]", res->GetFile(), res->GetExportedFile());

    App->fs->SplitFilePath(res->file.c_str(), nullptr, &res->user_name, nullptr);

    res->user_name = user_name;

    if (res->user_name.empty())
    {
        res->user_name = res->exported_file;
    }

    size_t pos_dot = res->user_name.find_last_of(".");
    if(pos_dot != std::string::npos)
    {
        res->user_name.erase(res->user_name.begin()+pos_dot, res->user_name.end());
    }

    return res->uid;
}

UID ModuleResources::ImportBuffer(const void * buffer, uint size, Resource::Type type, const char* source_file)
{
	UID ret = 0;

	bool import_ok = false;
	string output;


	switch (type)
	{
		case Resource::texture:
			import_ok = App->tex->Import(buffer, size, output, true);
		break;
		case Resource::mesh:
			// Old school trick: if it is a Mesh, buffer will be treated as an AiMesh*
			// TODO: this can go bad in so many ways :)
			// \todo: import_ok = App->meshes->Import((aiMesh*) buffer, output);
            
		break;
		case Resource::animation:
			// import_ok = ResourceAnimation::Import( anim_loader->Import((aiAnimation*) buffer, (UID) size, output);
		break;
	}

	// If export was successfull, create a new resource
	if (import_ok  == true)
	{
		Resource* res = CreateNewResource(type);
		if (source_file != nullptr) {
			res->file = source_file;
			App->fs->NormalizePath(res->file);
		}
		string file;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file);
		res->exported_file = file;
		ret = res->uid;
		LOG("Imported successful from BUFFER [%s] to [%s]", res->GetFile(), res->GetExportedFile());
	}
	else
		LOG("Importing of BUFFER [%s] FAILED", source_file);

	return ret;
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

Resource * ModuleResources::Get(UID uid) 
{			   
	std::map<UID, Resource*>::iterator it = resources.find(uid);
	if(it != resources.end())
		return it->second;
	return nullptr;
}

Resource * ModuleResources::CreateNewResource(Resource::Type type, UID force_uid)
{
	Resource* ret = nullptr;
	UID uid;

	if (force_uid != 0 && Get(force_uid) == nullptr)
		uid = force_uid;
	else
		uid = GenerateNewUID();

	switch (type)
	{
		case Resource::texture:
			ret = (Resource*) new ResourceTexture(uid);
		break;
		case Resource::mesh:
			ret = (Resource*) new ResourceMesh(uid);
		break;
		case Resource::audio:
			ret = (Resource*) new ResourceAudio(uid);
		break;
		case Resource::animation:
			ret = (Resource*) new ResourceAnimation(uid);
		break;
        case Resource::material:
            ret = new ResourceMaterial(uid);
        break;
        case Resource::model:
            ret= new ResourceModel(uid);
            break;
        case Resource::state_machine:
            ret= new ResourceStateMachine(uid);
            break;

    }

	if (ret != nullptr)
	{
		resources[uid] = ret;
	}

	return ret;
}

void ModuleResources::GatherResourceType(std::vector<const Resource*>& resources, Resource::Type type) const
{
	for (map<UID, Resource*>::const_iterator it = this->resources.begin(); it != this->resources.end(); ++it)
	{
		if (it->second->type == type)
			resources.push_back(it->second);
	}
}

void ModuleResources::LoadUID()
{
	string file(SETTINGS_FOLDER);
	file += LAST_UID_FILE;

	char *buf = nullptr;
	uint size = App->fs->Load(file.c_str(), &buf);

	if (size == sizeof(last_uid))
	{
		last_uid = *((UID*)buf);
		RELEASE_ARRAY(buf);
	}
	else
	{
		LOG("WARNING! Cannot read resource UID from file [%s] - Generating a new one", file.c_str());
		SaveUID();
	}
}

void ModuleResources::SaveUID() const
{
	string file(SETTINGS_FOLDER);
	file += LAST_UID_FILE;

	uint size = App->fs->Save(file.c_str(), (const char*) &last_uid, sizeof(last_uid));

	if (size != sizeof(last_uid))
		LOG("WARNING! Cannot write resource UID into file [%s]", file.c_str());
}

void ModuleResources::RemoveResource(UID uid)
{
    map<UID, Resource*>::iterator it = resources.find(uid);
    if(it != resources.end())
    {
        App->fs->Remove(it->second->GetExportedFile());

        char tmp[256];
        sprintf_s(tmp, 255, "%s%s", GetDirByType(it->second->GetType()), it->second->GetExportedFile());
        App->fs->Remove(tmp);

        removed.push_back(it->second);

        resources.erase(it);
    }
}

