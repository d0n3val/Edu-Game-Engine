#ifndef __APPLICATION_CPP__
#define __APPLICATION_CPP__

#include "Globals.h"
#include <list>
#include <string>
#include "Timer.h"

class Config;
class Module;
class ModuleHardware;
class ModuleFileSystem;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleSceneLoader;
class ModuleTextures;
class ModuleMeshes;
class ModuleEditor;
class ModuleLevelManager;
class ModuleResources;
struct Event;

class Application
{

public:
	Application();
	~Application();

	void ReadConfiguration(const Config& config);
	void SaveConfiguration(Config& config) const;

	bool Init();
	update_status Update();
	bool CleanUp();
	const char* GetAppName() const;
	void SetAppName(const char* name) ;
	const char* GetOrganizationName() const;
	void SetOrganizationName(const char* name) ;
	uint GetFramerateLimit() const;
	void SetFramerateLimit(uint max_framerate);
	void Log(const char* entry);
	void LoadPrefs(const char* file);
	void SavePrefs(const char* file);
	void RequestBrowser(const char* url) const;
	void BroadcastEvent(const Event& event);

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	ModuleHardware* hw = nullptr;
	ModuleFileSystem* fs = nullptr;
	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleAudio* audio = nullptr;
	ModulePhysics3D* physics3D = nullptr;
	ModuleRenderer3D* renderer3D = nullptr;
	ModuleCamera3D* camera = nullptr;
	ModuleSceneLoader* scene = nullptr;
	ModuleTextures* tex = nullptr;
	ModuleMeshes* meshes = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleLevelManager* level = nullptr;
	ModuleResources* resources = nullptr;

private:

	Timer	ms_timer;
	Timer	fps_timer;
	Uint32	frames;
	float	dt;
	int		fps_counter;
	int		last_frame_ms;
	int		last_fps;
	int		capped_ms;

	std::list<Module*> modules;
	std::string log;
	std::string app_name;
	std::string organization_name;
};

// Give App pointer access everywhere
extern Application* App;

#endif // __APPLICATION_CPP__