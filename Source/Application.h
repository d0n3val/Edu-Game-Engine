#ifndef __APPLICATION_CPP__
#define __APPLICATION_CPP__

#include "Globals.h"
#include <list>
#include <string>
#include "Timer.h"
#include "MathGeoLib/include/Algorithm/Random/LCG.h"

class Config;
class Module;
class ModuleHardware;
class ModuleFileSystem;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleEditorCamera;
class ModuleSceneLoader;
class ModuleTextures;
class ModuleMeshes;
class ModuleEditor;
class ModuleLevelManager;
class ModuleResources;
class ModuleAnimation;
class ModuleAI;

struct Event;

class Application
{
public:
	enum State
	{
		play,
		stop,
		pause,
		waiting_play,
		waiting_stop,
		waiting_pause,
		waiting_unpause
	};

public:
	Application();
	~Application();

	void ReadConfiguration(const Config& config);
	void SaveConfiguration(Config& config) const;

	bool Init();
	update_status Update();
	bool CleanUp();
	void DebugDraw();
	const char* GetAppName() const;
	void SetAppName(const char* name) ;
	const char* GetOrganizationName() const;
	void SetOrganizationName(const char* name) ;
	uint GetFramerateLimit() const;
	void SetFramerateLimit(uint max_framerate);
	void Log(const char* entry);
	void LoadPrefs();
	void SavePrefs() const;
	void RequestBrowser(const char* url) const;
	void BroadcastEvent(const Event& event);
	State GetState() const;
	void Play();
	void Pause();
	void UnPause();
	void Stop();
	bool IsPlay() const;
	bool IsPause() const;
	bool IsStop() const;

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	LCG*	random = nullptr;

	ModuleHardware* hw = nullptr;
	ModuleFileSystem* fs = nullptr;
	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleAudio* audio = nullptr;
	ModulePhysics3D* physics3D = nullptr;
	ModuleRenderer3D* renderer3D = nullptr;
	ModuleEditorCamera* camera = nullptr;
	ModuleSceneLoader* scene = nullptr;
	ModuleTextures* tex = nullptr;
	ModuleMeshes* meshes = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleLevelManager* level = nullptr;
	ModuleResources* resources = nullptr;
	ModuleAnimation* animation = nullptr;
	ModuleAI* ai = nullptr;

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

	State state = State::stop;
};

// Give App pointer access everywhere
extern Application* App;

#endif // __APPLICATION_CPP__