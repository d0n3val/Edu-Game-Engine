#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include <vector>

#define TEXTURE_PATH "/Library/Textures/"

struct TextureInfo
{
	char name[15];
	uint width, height;
	uint depth, bpp;
	uint mips;
	uint gpu_id;
};


class ModuleTextures : public Module
{
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	const TextureInfo* Load(const char* file);
	const char* Import(const char* file, const char* path);
	const char* Import(const void* buffer, uint size);

public:
	std::vector<TextureInfo* > textures;
};

#endif // __MODULETEXTURES_H__