#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Config.h"

using namespace std;

ModuleLevelManager::ModuleLevelManager( bool start_enabled) : Module("LevelManager", start_enabled)
{}

// Destructor
ModuleLevelManager::~ModuleLevelManager()
{}

// Called before render is available
bool ModuleLevelManager::Init(Config* config)
{
	bool ret = true;
	LOG("Loading Level Manager");
	
	return ret;
}

bool ModuleLevelManager::Start(Config * config)
{
	return true;
}

// Called before quitting
bool ModuleLevelManager::CleanUp()
{
	LOG("Freeing Level Manager");

	return true;
}

bool ModuleLevelManager::Load(const char * file)
{
	bool ret = false;

	if (file != nullptr)
	{
		int len = strlen(file);

		char* buffer = nullptr;
		uint size = App->fs->Load(file, &buffer);

		if (buffer != nullptr)
		{
		}

		RELEASE(buffer); // since we are not buffering the file, we can safely remove it
	}

	return ret;
}

bool ModuleLevelManager::Save(const char * file)
{
	Config test;

	test.CreateEmpty();
	Config c(test.AddSection("hello_world"));

	c.AddString("my name", "is ric");
	c.AddBool("my bool", true);
	c.AddInt("my int", 123);
	c.AddFloat("my float", 3.1416f);

	c.AddSection("Sebsection2");

	char* buf = nullptr;

	uint size = test.Save(&buf, "This is a test");

	App->fs->Save("prefs.json", buf, size);

	RELEASE(buf);

	return false;
}
