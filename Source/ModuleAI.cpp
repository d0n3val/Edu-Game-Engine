#include "Globals.h"
#include "Application.h"
#include "ModuleAI.h"
#include "Config.h"

using namespace std;

ModuleAI::ModuleAI( bool start_enabled) : Module("AI", start_enabled)
{}

// Destructor
ModuleAI::~ModuleAI()
{}

// Called before render is available
bool ModuleAI::Init(Config* config)
{
	bool ret = true;
	LOG("Loading AI");
	
	return ret;
}

bool ModuleAI::Start(Config * config)
{
	return true;
}

update_status ModuleAI::Update(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleAI::CleanUp()
{
	LOG("Freeing AI");

	return true;
}

void ModuleAI::Save(Config * config) const
{
}

void ModuleAI::Load(Config * config)
{
}
