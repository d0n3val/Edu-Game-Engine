#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

class ResourceTexture : public Resource
{
	friend class ModuleTextures;
public:

	enum Format {
		color_index,
		rgb,
		rgba,
		bgr,
		bgra,
		luminance,
		unknown
	};

public:

	ResourceTexture(UID id);
	virtual ~ResourceTexture();

	const char* GetFormatStr() const;

	bool LoadInMemory() override;
    void ReleaseFromMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

    uint GetID() const { return gpu_id; }
    uint GetWidth() const { return width; }
    uint GetHeight() const { return height; }
    uint GetDepth() const { return depth; }
    uint GetBPP() const { return bpp; }
    uint GetMips() const { return mips; }
    uint GetBytes() const { return bytes; }

private:
	uint width = 0;
	uint height = 0;
	uint depth = 0;
	uint bpp = 0;
	uint mips = 0;
	uint bytes = 0;
	uint gpu_id = 0;

	Format format = unknown;
};

#endif // __RESOURCE_TEXTURE_H__
