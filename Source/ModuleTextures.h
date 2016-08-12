#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include <vector>

struct TextureInfo
{
	char name[15];
	uint width = 0;
	uint height = 0;
	uint depth = 0;
	uint bpp = 0;
	uint mips = 0;
	uint bytes = 0;
	uint gpu_id = 0;
	enum {
		unknown,
		color_index,
		rgb,
		rgba,
		bgr,
		bgra,
		luminance
	} format = unknown;
};

static const char* texture_formats[] = { "unknown", "color index", "rgb", "rgba", "bgr", "bgra", "luminance" };

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