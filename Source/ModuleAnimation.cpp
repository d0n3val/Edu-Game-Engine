#include "Globals.h"
#include "Application.h"
#include "ModuleAnimation.h"

//#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

using namespace std;

ModuleAnimation::ModuleAnimation(bool start_active) : Module("Animation", start_active)
{
}

// Destructor
ModuleAnimation::~ModuleAnimation()
{
}

// Called before render is available
bool ModuleAnimation::Init(Config* config)
{
	LOG("Loading Animation System");
	bool ret = true;

	return ret;
}

// Called before quitting
bool ModuleAnimation::CleanUp()
{
	LOG("Freeing Animation subsystem");

	return true;
}