#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModulePhysics3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleMeshes.h"
#include "Config.h"

using namespace std;

// ---------------------------------------------
Application::Application()
{
	frames = 0;
	last_frame_ms = -1;
	last_fps = -1;
	capped_ms = 1000 / 60;
	fps_counter = 0;

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	modules.push_back(fs = new ModuleFileSystem(ASSETS_FOLDER));
	modules.push_back(window = new ModuleWindow());
	modules.push_back(tex = new ModuleTextures());
	modules.push_back(meshes = new ModuleMeshes());
	modules.push_back(physics3D = new ModulePhysics3D());
	modules.push_back(renderer3D = new ModuleRenderer3D());
	modules.push_back(camera = new ModuleCamera3D());
	modules.push_back(input = new ModuleInput());
	modules.push_back(audio = new ModuleAudio(true));
	modules.push_back(scene = new ModuleScene());
}

// ---------------------------------------------
Application::~Application()
{
	for(list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		RELEASE(*it);
}

void Application::ReadConfiguration(const Config& config)
{
	app_name = config.GetString("Name", "Edu Engine");
	organization_name = config.GetString("Organization", "UPC CITM");
}

// ---------------------------------------------
bool Application::Init()
{
	bool ret = true;
			
	char* buffer = nullptr;
	fs->Load("config.json", &buffer);

	Config config;
	config.CreateFromString((const char*) buffer);

	ReadConfiguration(config);

	// We init everything, even if not anabled
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
	{
		ret = (*it)->Init(config.IsValid() ? &(config.GetSection((*it)->GetName())) : nullptr); 
	}

	// Another round, just before starting the Updates. Only called for "active" modules
	// we send the configuration again in case a module needs it
	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
	{
		if((*it)->IsEnabled() == true)
			ret = (*it)->Start(config.IsValid() ? &(config.GetSection((*it)->GetName())) : nullptr); 
	}

	audio->LoadFx("Assets/audio/effects/23H.wav");
	RELEASE(buffer);
	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();
}

// ---------------------------------------------
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();

	if(input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		audio->PlayFx(0);
	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsEnabled() == true) 
			ret = (*it)->PreUpdate(dt);

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsEnabled() == true) 
			ret = (*it)->Update(dt);

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsEnabled() == true) 
			ret = (*it)->PostUpdate(dt);

	FinishUpdate();
	return ret;
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	// Recap on framecount and fps
	++frames;
	++fps_counter;

	if(fps_timer.Read() >= 1000)
	{
		last_fps = fps_counter;
		fps_counter = 0;
		fps_timer.Start();
	}

	last_frame_ms = ms_timer.Read();

	// cap fps
	if(last_frame_ms < capped_ms)
	{
		SDL_Delay(capped_ms - last_frame_ms);
	}

	char t[150];
	sprintf_s(t, "%s FPS: %d Camera: %0.1f,%0.1f,%0.1f", app_name.c_str(), (int)last_fps,
		camera->Position.x, camera->Position.y, camera->Position.z);
	window->SetTitle(t);
}

// ---------------------------------------------
bool Application::CleanUp()
{
	bool ret = true;

	for(list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if((*it)->IsEnabled() == true) 
			ret = (*it)->CleanUp();

	return ret;
}

// ---------------------------------------------
const char* Application::GetAppName() const
{
	return app_name.c_str();
}

// ---------------------------------------------
const char* Application::GetOrganizationName() const
{
	return organization_name.c_str();
}