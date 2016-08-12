#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include <vector>

class ResourceTexture;

class ModuleTextures : public Module
{
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Load(ResourceTexture* resource);
	// TODO: Unload Textures
	const char* Import(const char* file, const char* path);
	const char* Import(const void* buffer, uint size);
};

#endif // __MODULETEXTURES_H__