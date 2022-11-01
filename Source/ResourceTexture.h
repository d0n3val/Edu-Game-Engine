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
		rgba = 0,
		bgra,
		bgr,
		bc1,
		bc3,
		bc4,
		bc5,
		bc6s,
		bc6u,
		bc7,
        unknown
	};

	enum ColorSpace
	{
		gamma,
		linear
	};

	enum TextureType
	{
		Texture2D = 0,
		TextureCube = 1
	};

public:

	ResourceTexture(UID id);
	virtual ~ResourceTexture();

	const char*      GetFormatStr 		() const;

	bool             LoadInMemory 		() override;
    void             ReleaseFromMemory 	() override;

    bool             Save         		();
	void             Save         		(Config& config) const override;
	void             Load         		(const Config& config) override;

    Texture*		 GetTexture   		() const { return glTexture.get(); }
    uint             GetID        		() const { return glTexture ? uint(glTexture->Id()) : uint(0); }
    uint             GetWidth     		() const { return width; }
    uint             GetHeight    		() const { return height; }
    uint             GetDepth     		() const { return depth; }
	ColorSpace 		 GetColorSpace 		() const { return colorSpace; }
	Format 		 	 GetFormat    		() const { return format; }
    uint             GetGLInternalFormat() const;
    TextureType      GetTexType   		() const { return textype; }
	
    bool             GetMipmaps 		() const { return mipMaps;  }
    void 			 GenerateMipmaps	(bool generate);
	void 		 	 SetColorSpace 		(ColorSpace space) { colorSpace = space; }
    bool             IsCompressed 		() const;

	bool 			 LoadToArray 		(Texture2DArray* texArray, uint index) const;

    static Type GetClassType() { return texture; }


private:

    uint GetGLFormat() const;
	uint GetGLType() const;

private:

	uint width      = 0;
	uint height     = 0;
	uint depth      = 0;
	uint arraySize  = 0;
    bool mipMaps = false;
	ColorSpace colorSpace = gamma;
	Format format   = rgba;
	TextureType textype = Texture2D;

    std::unique_ptr<Texture> glTexture;

};

#endif // __RESOURCE_TEXTURE_H__
