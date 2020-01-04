#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"
#include "OGL.h"
#include "utils/SimpleBinStream.h"

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

	const char*      GetFormatStr() const;

	bool             LoadInMemory() override;
    void             ReleaseFromMemory() override;

    bool             Save         ();
	void             Save         (Config& config) const override;
	void             Load         (const Config& config) override;

    Texture2D*		 GetTexture   () const { return texture.get(); }
    uint             GetID        () const { return texture ? texture->Id() : 0; }
    uint             GetWidth     () const { return width; }
    uint             GetHeight    () const { return height; }
    uint             GetDepth     () const { return depth; }
    uint             GetBPP       () const { return bpp; }
    bool             HasMips      () const { return has_mips; }
    uint             GetBytes     () const { return bytes; }
    bool             GetLinear    () const { return linear; }
    bool             GetCompressed() const { return compressed; }

    void             SetLinear    (bool l);
    void             EnableMips   (bool enable);

private:

	uint width      = 0;
	uint height     = 0;
	uint depth      = 0;
	uint bpp        = 0;
	bool has_mips   = false;
    bool linear     = true;
    bool compressed = true;
	uint bytes      = 0;
	Format format   = unknown;

    std::unique_ptr<Texture2D> texture;

};

#endif // __RESOURCE_TEXTURE_H__
