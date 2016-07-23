#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc130-mt.lib")

using namespace std;

ModuleScene::ModuleScene( bool start_enabled) : Module( start_enabled)
{}

// Destructor
ModuleScene::~ModuleScene()
{}

// Called before render is available
bool ModuleScene::Init()
{
	LOG("Loading Scene Manager");
	bool ret = true;
	struct aiLogStream stream;

	// Stream log messages to Debug window
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return ret;
}

bool ModuleScene::LoadScene(const char* file)
{
	bool ret = true;


	return ret;
}

// Called before quitting
bool ModuleScene::CleanUp()
{
	LOG("Freeing Scene Manager");

	// Clean scene data
	if(scene != nullptr)
		aiReleaseImport(scene);

	// detach log stream
	aiDetachAllLogStreams();

	return true;
}
