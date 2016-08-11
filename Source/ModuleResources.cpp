#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "Event.h"
#include <string>

using namespace std;

ModuleResources::ModuleResources(bool start_enabled) : Module("Resource Manager", start_enabled), asset_folder(ASSET_FOLDER)
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

bool ModuleResources::ImportFile(const char * full_path, const char * destination)
{
	string final_path;
	if (destination == nullptr)
	{
		App->fs->SplitFilePath(full_path, nullptr, &final_path, nullptr);
		final_path = asset_folder + final_path;
	}
	else
		final_path = asset_folder + destination;

	App->fs->Copy(full_path, final_path.c_str());

	return false;
}
