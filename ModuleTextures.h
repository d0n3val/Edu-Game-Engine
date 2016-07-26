#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include <string>
#include <map>

struct SDL_Texture;

#define INVALID_TEXTURE 65535

class ModuleTextures : public Module
{
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr);
	bool CleanUp();

	uint Load(const char* file, const char* path);
	uint GetId(const char* path);

private:
	std::map<std::string, uint> textures;
};

#endif // __MODULETEXTURES_H__