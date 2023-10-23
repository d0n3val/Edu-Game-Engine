#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"
#include "OGL.h"
#include "utils/SimpleBinStream.h"

#include <optional>

enum TextureFormat
{
    Texture_rgba = 0,
    Texture_rgba32f,
    Texture_bgra,
    Texture_bgr,
    Texture_red,
    Texture_bc1,
    Texture_bc3,
    Texture_bc4,
    Texture_bc5,
    Texture_bc6s,
    Texture_bc6u,
    Texture_bc7,
    Texture_unknown
};

enum CompressType
{
    Compress_Colour,
    Compress_Grayscale,
    Compress_Normals,
    Compress_HDR,
    Compress_Colour_HQ,
    Compress_Colour_HQ_Fast,

    /*
    Compress_BC1 = 0, // Color RGB
    Compress_BC3, // Color RGBA
    Compress_BC4, // Greyscale
    Compress_BC5, // Two Greayscale (normals?)
    Compress_BC6, // half-floats
    Compress_BC7, // Color RGB or RGBA high quality
    Compress_BC7_FAST,  // Prev Faster
    */
    Compress_Count
};

enum TextureType
{
    TextureType_1D = 0,
    TextureType_2D,
    TextureType_3D,
    TextureType_Cube
};

enum ColorSpace
{
    ColorSpace_gamma,
    ColorSpace_linear
};

struct TextureMetadata
{
    TextureFormat format = Texture_rgba;
    TextureType texType = TextureType_2D;
    uint width = 0;
    uint height = 0;
    uint depth = 0;
    uint arraySize = 0;
    uint memSize = 0;
    uint mipCount = 0;
};

class ResourceTexture : public Resource
{
	friend class ModuleTextures;
    friend class ModuleResources;
public:


public:

	ResourceTexture(UID id);
	virtual ~ResourceTexture();

	const char*             GetFormatStr 		() const;

	bool                    LoadInMemory 		() override;
    void                    ReleaseFromMemory 	() override;

    bool                    Save            ();
	void                    Save         	(Config& config) const override;
	void                    Load         	(const Config& config) override;

    Texture*                GetTexture   	() const { return glTexture.get(); }
    uint                    GetID        	() const { return glTexture ? uint(glTexture->Id()) : uint(0); }
	ColorSpace              GetColorSpace   () const { return colorSpace.value_or(formatColorSpace);; }
    const TextureMetadata&  GetMetadata     () const {return metadata;}
	
    void                    SetColorSpace   (ColorSpace space) { colorSpace = space;}
    
	static bool             Import(const char* file, std::string& output_file, bool generateCubemap, bool generateMipmaps);
	static bool             Import(const void* buffer, uint size, std::string& output_file, bool toCubemap, bool generateMipmaps);

    bool                    LoadFromBuffer(const void* buffer, uint size, ColorSpace space);
    bool                    LoadCheckers();
    bool                    LoadFallback(ResourceTexture* resource, const float3& color);
    bool                    LoadRedImage(ResourceTexture* resource, uint widht, uint height);


    static Type GetClassType() { return texture; }


private:

	static bool ImportNoConvert(const void* buffer, uint size, std::string& output_file, bool generateMipmaps);
    static bool ImportToCubemap(const void* buffer, uint size, std::string& output_file, bool generateMipmaps);

    static Texture* TextureFromMemory(const void* buffer, uint size, TextureMetadata& metadata, ColorSpace space);

private:

    TextureMetadata metadata;
    std::optional<ColorSpace> colorSpace;
    ColorSpace formatColorSpace;

    std::unique_ptr<Texture> glTexture;
};

#endif // __RESOURCE_TEXTURE_H__
