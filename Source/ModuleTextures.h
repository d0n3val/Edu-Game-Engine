#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include "Math.h"

class ResourceTexture;
class Texture2DArray;

class ModuleTextures : public Module
{
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Load(ResourceTexture* resource);
    bool LoadToArray(const ResourceTexture* resource, Texture2DArray* texture, uint index);

	// TODO: Unload Texture

    bool ImportCube(const char* files [], const char* path, std::string& output_file, bool compressed);
	bool Import(const char* file, const char* path, std::string& output_file, bool compressed);
	bool Import(const void* buffer, uint size, std::string& output_file, bool compressed);

	bool LoadCheckers(ResourceTexture* resource);
    bool LoadFallback(ResourceTexture* resource, const float3& color);
    bool LoadCube(ResourceTexture* resource, const char* files [], const char* path);

private:
	bool Import(const void* buffer, uint size, bool compressed, uint header_size, void*& output_buffer, uint& output_size);
};

#endif // __MODULETEXTURES_H__
