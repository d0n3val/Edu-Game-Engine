#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Globals.h"
#include "Module.h"
#include "CubemapUtils.h"
#include "Math.h"

class ResourceTexture;
class Texture2DArray;

class ModuleTextures : public Module
{
    CubemapUtils cubeUtils;
public:
	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(Config* config = nullptr) override;
	bool CleanUp() override;

	bool Load(ResourceTexture* resource);
    bool LoadToArray(const ResourceTexture* resource, Texture2DArray* texture, uint index);

	// TODO: Unload Texture

    bool ImportCube(const std::string files [], std::string& output_file, bool compressed);
	bool Import(const char* file, std::string& output_file, bool compressed, bool toCubemap);
	bool Import(const void* buffer, uint size, std::string& output_file, bool compressed, bool toCubemap);

	bool LoadCheckers(ResourceTexture* resource);
    bool LoadFallback(ResourceTexture* resource, const float3& color);
    bool LoadCube(ResourceTexture* resource, const char* files [], const char* path);

private:
	bool Import(const void* buffer, uint size, bool compressed, uint header_size, void*& output_buffer, uint& output_size);
	bool Import(const void* buffer, uint size, std::string& output_file, bool compressed);
    bool ImportToCubemap(const void* buffer, uint size, std::string& output_file);
};

#endif // __MODULETEXTURES_H__
