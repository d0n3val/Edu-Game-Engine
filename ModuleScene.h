#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include <vector>
#include "Module.h"

struct aiScene;

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init();
	bool CleanUp();

	bool LoadScene(const char* file);

private:

	const struct aiScene* scene = nullptr;
};

#endif // __MODULE_SCENE_H__