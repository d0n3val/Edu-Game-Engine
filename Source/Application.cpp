#include "Application.h"
#include "ModuleHardware.h"
#include "ModuleFileSystem.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModulePhysics3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ModuleEditor.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "ModuleRenderer.h"
#include "ModuleHints.h"
#include "ModuleDebugDraw.h"
#include "ModuleAI.h"
#include "Event.h"
#include "Config.h"
#include "ThreadPool.h"
#include "GameObject.h"

using namespace std;

// ---------------------------------------------
Application::Application()
{
    threadPool = std::make_unique<ThreadPool>();

    frames = 0;
	last_frame_ms = -1;
	last_fps = -1;
	capped_ms = 1000 / 60;
	fps_counter = 0;
	random = new math::LCG();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	modules.push_back(hints = new ModuleHints());
	modules.push_back(hw = new ModuleHardware(false));
	modules.push_back(fs = new ModuleFileSystem(ASSETS_FOLDER));
	modules.push_back(window = new ModuleWindow());
	modules.push_back(resources = new ModuleResources());
	modules.push_back(physics3D = new ModulePhysics3D());
	modules.push_back(camera = new ModuleEditorCamera());
	modules.push_back(renderer3D = new ModuleRenderer3D());
	modules.push_back(input = new ModuleInput());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(audio = new ModuleAudio(true));
	modules.push_back(ai = new ModuleAI());
	modules.push_back(level = new ModuleLevelManager());
    modules.push_back(programs = new ModulePrograms(true));
    modules.push_back(renderer = new ModuleRenderer());
	modules.push_back(debug_draw = new ModuleDebugDraw());
}

// ---------------------------------------------
Application::~Application()
{
	for(list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); ++it)
		RELEASE(*it);

	RELEASE(random);
}

void Application::ReadConfiguration(const Config& config)
{
	app_name = config.GetString("Name", "Edu Engine");
	organization_name = config.GetString("Organization", "UPC CITM");
	SetFramerateLimit(config.GetInt("MaxFramerate", 0));
}

void Application::SaveConfiguration(Config& config) const
{
	config.AddString("Name", app_name.c_str());
	config.AddString("Organization", organization_name.c_str());
	config.AddInt("MaxFramerate", GetFramerateLimit());
}

// ---------------------------------------------
bool Application::Init()
{
	bool ret = true;

	char* buffer = nullptr;
	fs->Load(SETTINGS_FOLDER "config.json", &buffer);

	Config config((const char*) buffer);

	ReadConfiguration(config.GetSection("App"));

	// We init everything, even if not anabled
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
	{
        Config section = config.GetSection((*it)->GetName());
		ret = (*it)->Init(&section); 
	}

	// Another round, just before starting the Updates. Only called for "active" modules
	// we send the configuration again in case a module needs it
	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
	{
        if ((*it)->IsActive() == true)
        {
            Config section = config.GetSection((*it)->GetName());
            ret = (*it)->Start(&section);
        }
	}

	RELEASE_ARRAY(buffer);
	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();

	switch (state)
	{
		case waiting_play:
		{
			state = play;
			BroadcastEvent(Event(Event::EventType::play));
		} break;
		case waiting_stop:
		{
			state = stop;
			BroadcastEvent(Event(Event::EventType::stop));
		} break;
		case waiting_pause:
		{
			state = pause;
			BroadcastEvent(Event(Event::EventType::pause));
		} break;
		case waiting_unpause:
		{
			state = play;
			BroadcastEvent(Event(Event::EventType::unpause));
		} break;
	}
}

// ---------------------------------------------
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsActive() == true) 
			ret = (*it)->PreUpdate(dt);

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsActive() == true) 
			ret = (*it)->Update(dt);

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		if((*it)->IsActive() == true) 
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
	if(capped_ms > 0 && (last_frame_ms < capped_ms))
		SDL_Delay(capped_ms - last_frame_ms);

	// notify the editor
	editor->LogFPS((float) last_fps, (float) last_frame_ms);
}

// ---------------------------------------------
bool Application::CleanUp()
{
	bool ret = true;

    fs->Save(SETTINGS_FOLDER "Engine.log", log.c_str(), uint(log.size()));
	SavePrefs();

	for(list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
		if((*it)->IsActive() == true) 
			ret = (*it)->CleanUp();

	return ret;
}

// ---------------------------------------------
void Application::DebugDraw()
{
	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		if ((*it)->IsActive() == true)
			(*it)->DrawDebug();
}

// ---------------------------------------------
const char* Application::GetAppName() const
{
	return app_name.c_str();
}

// ---------------------------------------------
void Application::SetAppName(const char * name)
{
	if (name != nullptr && name != app_name)
	{
		app_name = name;
		window->SetTitle(name);
		// TODO: Filesystem should adjust its writing folder
	}
}

// ---------------------------------------------
const char* Application::GetOrganizationName() const
{
	return organization_name.c_str();
}

void Application::SetOrganizationName(const char * name)
{
	if (name != nullptr && name != organization_name)
	{
		organization_name = name;
		// TODO: Filesystem should adjust its writing folder
	}
}

// ---------------------------------------------
uint Application::GetFramerateLimit() const
{
	if(capped_ms > 0)
		return (uint) ((1.0f/(float)capped_ms) * 1000.0f);
	else
		return 0;
}

// ---------------------------------------------
void Application::SetFramerateLimit(uint max_framerate)
{				  
	if (max_framerate > 0)
		capped_ms = 1000 / max_framerate;
	else
		capped_ms = 0;
}

// ---------------------------------------------
void Application::Log(const char * entry)
{
	// save all logs, so we can dump all in a file upon close
	log.append(entry);

	// send to editor console
	editor->Log(entry);
}

// ---------------------------------------------
void Application::LoadPrefs()
{
	char* buffer = nullptr;
	fs->Load(SETTINGS_FOLDER "config.json", &buffer);

	if (buffer != nullptr)
	{
		Config config((const char*)buffer);

		if (config.IsValid() == true)
		{
			LOG("Loading Engine Preferences");

			ReadConfiguration(config.GetSection("App"));

			Config section;
			for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
			{
				section = config.GetSection((*it)->GetName());
				//if (section.IsValid())
					(*it)->Load(&section);
			}
		}
		else
			LOG("Cannot load Engine Preferences: Invalid format");

		RELEASE_ARRAY(buffer);
	}
}

// ---------------------------------------------
void Application::SavePrefs() const
{
	Config config;

    Config appCfg = config.AddSection("App");
	SaveConfiguration(appCfg);

    for (list<Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it)
    {
        Config section = config.AddSection((*it)->GetName());
        (*it)->Save(&section);
    }

	char *buf;
	uint size = uint(config.Save(&buf, "Saved preferences for Edu Engine"));
	if(App->fs->Save(SETTINGS_FOLDER "config.json", buf, size) > 0)
		LOG("Saved Engine Preferences");
	RELEASE_ARRAY(buf);
}

// ---------------------------------------------
void Application::RequestBrowser(const char * url) const
{
   ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

// ---------------------------------------------
void Application::BroadcastEvent(const Event& event)
{
	for (list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
		(*it)->ReceiveEvent(event);
}

// ---------------------------------------------
Application::State Application::GetState() const
{
	return state;
}

// ---------------------------------------------
LCG & Application::Random()
{
	return *random;
}

// ---------------------------------------------
void Application::Play()
{
	if (state == State::stop)
		state = State::waiting_play;
}

// ---------------------------------------------
void Application::Pause()
{
	if (state == State::play)
		state = State::waiting_pause;
}

// ---------------------------------------------
void Application::UnPause()
{
	if (state == State::pause)
		state = State::waiting_unpause;
}

// ---------------------------------------------
void Application::Stop()
{
	if (state == State::play || state == State::pause)
		state = State::waiting_stop;
}

bool Application::IsPlay() const
{
	return state == State::play;
}

bool Application::IsPause() const
{
	return state == State::pause;
}

bool Application::IsStop() const
{
	return state == State::stop;
}
