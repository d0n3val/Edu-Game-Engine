#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include <string>
#include <map>

class ModuleTextures : public Module
{
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	const char* Import(const char* file, const char* path);
	const char* Import(const void* buffer, uint size);
	uint Load(const char* file, const char* path);
	uint Load(const void* buffer, uint size);
	uint GetId(const char* path);

private:
	std::map<std::string, uint> textures;
};

#endif // __MODULETEXTURES_H__