#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

class ResourceTexture : public Resource
{
	friend class ModuleTextures;

public:
	ResourceTexture(UID id);
	virtual ~ResourceTexture();

	const char* GetFormatStr() const;

public:
	uint width = 0;
	uint height = 0;
	uint depth = 0;
	uint bpp = 0;
	uint mips = 0;
	uint bytes = 0;
	uint gpu_id = 0;

	enum {
		color_index,
		rgb,
		rgba,
		bgr,
		bgra,
		luminance,
		unknown
	} format = unknown;
};

#endif // __RESOURCE_TEXTURE_H__