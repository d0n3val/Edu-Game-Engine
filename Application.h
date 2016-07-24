#ifndef __APPLICATION_CPP__
#define __APPLICATION_CPP__

#include "Globals.h"
#include <list>
#include "Timer.h"
#include "Module.h"

class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePlayer;
class ModuleSceneIntro;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleScene;
class ModuleTextures;
class ModuleMeshes;

class Application
{
public:
	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleAudio* audio = nullptr;
	ModulePlayer* player = nullptr;
	ModuleSceneIntro* scene_intro = nullptr;
	ModulePhysics3D* physics3D = nullptr;
	ModuleRenderer3D* renderer3D = nullptr;
	ModuleCamera3D* camera = nullptr;
	ModuleScene* scene = nullptr;
	ModuleTextures* tex = nullptr;
	ModuleMeshes* meshes = nullptr;

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

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

private:

	void PrepareUpdate();
	void FinishUpdate();
};

// Give App pointer access everywhere
extern Application* App;

#endif // __APPLICATION_CPP__