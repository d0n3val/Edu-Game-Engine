#ifndef __APPLICATION_CPP__
#define __APPLICATION_CPP__

#include "Globals.h"
#include <list>
#include <string>
#include "Timer.h"

class Config;
class Module;
class ModuleFileSystem;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleScene;
class ModuleTextures;
class ModuleMeshes;
class ModuleEditor;
class ModuleLevelManager;

class Application
{
public:
	ModuleFileSystem* fs = nullptr;
	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleAudio* audio = nullptr;
	ModulePhysics3D* physics3D = nullptr;
	ModuleRenderer3D* renderer3D = nullptr;
	ModuleCamera3D* camera = nullptr;
	ModuleScene* scene = nullptr;
	ModuleTextures* tex = nullptr;
	ModuleMeshes* meshes = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleLevelManager* level = nullptr;

private:

	Timer	ms_timer;
	Timer	fps_timer;
	Uint32	frames;
	float	dt;
	int		fps_counter;
	int		last_frame_ms;
	int		last_fps;
	int		capped_ms;

	std::string app_name;
	std::string organization_name;
	std::list<Module*> modules;

public:

	Application();
	~Application();

	void ReadConfiguration(const Config& config);

	bool Init();
	update_status Update();
	bool CleanUp();
	const char* GetAppName() const;
	const char* GetOrganizationName() const;
	uint GetFramerateLimit() const;
	void SetFramerateLimit(uint max_framerate);
	void Log(const char* entry);
	void OnResize(uint width, uint height);
	void LoadPrefs();
	void SavePrefs();

private:

	void PrepareUpdate();
	void FinishUpdate();

private:
	std::string log;
};

// Give App pointer access everywhere
extern Application* App;

#endif // __APPLICATION_CPP__