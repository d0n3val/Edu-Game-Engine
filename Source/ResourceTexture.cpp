#include "ResourceTexture.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "CubemapUtils.h"
#include "Config.h"


#include "DirectXTex/DirectXTex.h"

#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include <memory>
#include <assert.h>

#ifdef max
#undef max
#endif 

//  Create checkerboard texture  
#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

namespace
{
    TextureFormat GetFormatFromDXGI(DXGI_FORMAT format, ColorSpace& colorSpace)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            colorSpace = ColorSpace_gamma;
            return Texture_rgba;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_rgba;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            colorSpace = ColorSpace_linear;
            return Texture_rgba32f;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            colorSpace = ColorSpace_gamma;
            return Texture_bgra;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bgra;
        case DXGI_FORMAT_B5G6R5_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bgr;
        case DXGI_FORMAT_R8_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_red;
        case DXGI_FORMAT_BC1_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bc1;
        case DXGI_FORMAT_BC3_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bc3;
        case DXGI_FORMAT_BC4_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bc4;
        case DXGI_FORMAT_BC5_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bc5;
        case DXGI_FORMAT_BC6H_SF16:
            colorSpace = ColorSpace_linear;
            return Texture_bc6s;
        case DXGI_FORMAT_BC6H_UF16:
            colorSpace = ColorSpace_linear;
            return Texture_bc6u;
        case DXGI_FORMAT_BC7_UNORM:
            colorSpace = ColorSpace_linear;
            return Texture_bc7;
        }

        return Texture_unknown;
    }

    bool IsCompressed(TextureFormat format) 
    {
        return format == Texture_bc1  ||
               format == Texture_bc3  ||
               format == Texture_bc4  ||
               format == Texture_bc5  ||
               format == Texture_bc6s ||
               format == Texture_bc6u ||
               format == Texture_bc7;
    }

    uint GetGLInternalFormat(TextureFormat format, ColorSpace colorSpace)
    {
        static uint gl_internal[]       = { GL_RGBA8, GL_RGBA32F, GL_RGBA8, GL_RGB8, GL_R8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM };
        static uint gl_internal_gamma[] = { GL_SRGB8_ALPHA8, GL_RGBA32F, GL_SRGB8_ALPHA8 , GL_SRGB8, GL_R8, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_RG_RGTC2 , GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT , GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM , GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM };

        return colorSpace == ColorSpace_linear ? gl_internal[uint(format)] : gl_internal_gamma[uint(format)];
    }

    uint GetGLFormat(TextureFormat format) 
    {
        static uint gl_format[] = { GL_RGBA, GL_RGBA, GL_BGRA, GL_BGR, GL_RED };
        assert(uint(format) < 5);

        return gl_format[uint(format)];
    }

    uint GetGLType(TextureFormat format) 
    {
        static uint gl_type[] = { GL_UNSIGNED_BYTE, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE };
        assert(uint(format) < 5);

        return gl_type[uint(format)];
    }
}

// ---------------------------------------------------------
ResourceTexture::ResourceTexture(UID uid) : Resource(uid, Resource::Type::texture)
{
}

// ---------------------------------------------------------
ResourceTexture::~ResourceTexture()
{}

// ---------------------------------------------------------
const char * ResourceTexture::GetFormatStr() const
{
	static const char* formats[] = { "RGBA", "RGBA", "BGRA", "BGR", "RED", "BC1", "BC3", "BC4", "BC5", "BC6s", "BC6u", "BC7"};

	return formats[metadata.format];
}

// ---------------------------------------------------------
bool ResourceTexture::LoadInMemory()
{
	char* head_buffer = nullptr;
	uint total_size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, GetExportedFile(), &head_buffer);

    bool ret = head_buffer != nullptr && total_size > 0;

    if (ret)
	{
        char *buffer = head_buffer + sizeof(uint32_t);

        glTexture.reset(TextureFromMemory(buffer, total_size - sizeof(uint32_t), metadata, formatColorSpace, colorSpace));
    }

    RELEASE_ARRAY(head_buffer);
    return ret;
}


// ---------------------------------------------------------
void ResourceTexture::ReleaseFromMemory() 
{
    glTexture.reset(nullptr);
}


// ---------------------------------------------------------
bool ResourceTexture::Save() 
{
	char* buffer = nullptr;

    uint size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, GetExportedFile(), &buffer);

    bool ok = size > 0 && buffer != nullptr;

    if(ok)
    {
        assert(exported_file.length() > 0);

        char full_path[250];

        sprintf_s(full_path, 250, "%s%s", LIBRARY_TEXTURES_FOLDER, exported_file.c_str());

        ok = App->fs->Save(full_path, buffer, size) > 0;

        RELEASE_ARRAY(buffer);
    }

    return ok;
}

// ---------------------------------------------------------
void ResourceTexture::Save(Config & config) const
{
	Resource::Save(config);

    if (colorSpace.has_value())
    {
        config.AddInt("ColorSpace", colorSpace.value());
    }
}

// ---------------------------------------------------------
void ResourceTexture::Load(const Config & config)
{
	Resource::Load(config);

    if(config.HasInt("ColorSpace"))
    {
        colorSpace = (ColorSpace)config.GetInt("ColorSpace", formatColorSpace);
    }

}

bool ResourceTexture::Import(const char* file, std::string& output_file, bool generateCubemap, bool generateMipmaps)
{
	std::string sFile(file);

	char* buffer = nullptr;
	uint size = App->fs->Load(file, &buffer);

    bool ret = Import(buffer, size, output_file, generateCubemap, generateMipmaps);

    if (ret == false)
    {
        LOG("Cannot load texture %s", file);
    }

    return ret;
}

bool ResourceTexture::Import(const void * buffer, uint size, std::string& output_file, bool generateCubemap, bool generateMipmaps)
{
    return generateCubemap ? ImportToCubemap(buffer, size, output_file, generateMipmaps) : ImportNoConvert(buffer, size, output_file, generateMipmaps);
}

bool ResourceTexture::ImportNoConvert(const void * buffer, uint size, std::string& output_file, bool generateMipmaps)
{   
	bool ret = buffer != nullptr;

    void *output_buffer = nullptr;
    uint output_size = 0;
    uint header_size = sizeof(uint32_t);

    if (ret)
    {

        DirectX::ScratchImage image, imageMips;
        DirectX::ScratchImage* result = &image;
        HRESULT res = DirectX::LoadFromDDSMemory(buffer, size, DirectX::DDS_FLAGS_NONE, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromHDRMemory(buffer, size, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromTGAMemory(buffer, size, DirectX::TGA_FLAGS_NONE, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromWICMemory(buffer, size, DirectX::WIC_FLAGS_NONE, nullptr, image);

        if (res == S_OK && generateMipmaps)
        {
            uint32_t levels = uint32_t(std::log2(std::max(result->GetMetadata().width, result->GetMetadata().height)));
            res = DirectX::GenerateMipMaps(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::TEX_FILTER_DEFAULT, levels, imageMips);
            result = &imageMips;
        }

        if (res == S_OK)
        {
            DirectX::Blob blob;            
            res = DirectX::SaveToDDSMemory(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::DDS_FLAGS_NONE, blob);
            
            if (res == S_OK)
            {
                output_size = uint(blob.GetBufferSize());
                output_buffer = new char[blob.GetBufferSize()+header_size];
                memcpy(&reinterpret_cast<char*>(output_buffer)[header_size], blob.GetBufferPointer(), blob.GetBufferSize());
            }
        }

        ret = res == S_OK;
    }

    if (ret)
    {
        *reinterpret_cast<uint32_t*>(output_buffer) = uint32_t(TextureType_2D);
        ret = App->fs->SaveUnique(output_file, output_buffer, output_size + header_size, LIBRARY_TEXTURES_FOLDER, "texture", "tex");

        RELEASE_ARRAY(buffer);
        RELEASE_ARRAY(output_buffer);
    }

	return ret;
}

bool ResourceTexture::ImportToCubemap(const void* buffer, uint size, std::string& output_file, bool generateMipmaps)
{
	bool ret = buffer  != nullptr;
    uint8_t* output_buffer = nullptr;
    uint output_size    = 0;

    TextureMetadata metadata;
    ColorSpace formatColorSpace;
    Texture* texture = TextureFromMemory(buffer, size, metadata, formatColorSpace, std::optional<ColorSpace>());

    if(metadata.texType == TextureType_2D)
    {
        CubemapUtils cubeUtils;
        std::unique_ptr<TextureCube> cubeMap(cubeUtils.ConvertToCubemap(static_cast<Texture2D *>(texture), 512, 512));

        output_size = sizeof(uint32_t);

        std::vector<float> tmpBuffer;

        tmpBuffer.resize(512 * 512 * 4);

        DirectX::ScratchImage image, imageMips;
        DirectX::ScratchImage* result = &image;

        image.InitializeCube(DXGI_FORMAT_R32G32B32A32_FLOAT, 512, 512, 1, 1);

        for (uint i = 0; i < 6; ++i)
        {
            cubeMap->GetData(i, 0, GL_RGBA, GL_FLOAT, &tmpBuffer[0]);

            const DirectX::Image* face = image.GetImage(0, i, 0);
            memcpy(face->pixels, &tmpBuffer[0], tmpBuffer.size() * sizeof(float));
        }

        if (generateMipmaps)
        {
            uint32_t levels = uint32_t(std::log2(std::max(result->GetMetadata().width, result->GetMetadata().height)));
            DirectX::GenerateMipMaps(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::TEX_FILTER_DEFAULT, levels, imageMips);
            result = &imageMips;
        }

        DirectX::Blob blob;
        ret = DirectX::SaveToDDSMemory(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::DDS_FLAGS_NONE, blob) == S_OK;

        if (ret)
        {
            uint header_size = sizeof(uint32_t);
            output_size = uint(blob.GetBufferSize()+header_size);
            output_buffer = new uint8_t[blob.GetBufferSize() + header_size];

            *(uint32_t*)output_buffer = uint32_t(TextureType_Cube);
            memcpy(&reinterpret_cast<char*>(output_buffer)[header_size], blob.GetBufferPointer(), blob.GetBufferSize());
        }
    }

    if(ret)
    {
        ret = App->fs->SaveUnique(output_file, output_buffer, output_size, LIBRARY_TEXTURES_FOLDER, "texture", "tex");

        RELEASE_ARRAY(buffer);
        RELEASE_ARRAY(output_buffer);
    }

    delete texture;

	return ret;
}

bool ResourceTexture::LoadFromBuffer(const void* buffer, uint size)
{
    glTexture.reset(TextureFromMemory(buffer, size, metadata, formatColorSpace, colorSpace));

    return true;
}

Texture* ResourceTexture::TextureFromMemory(const void *buffer, uint size, TextureMetadata& metadata, ColorSpace& formatSpace, std::optional<ColorSpace> colorSpace)
{
    DirectX::ScratchImage image;
    Texture* result = nullptr;

    HRESULT res = DirectX::LoadFromDDSMemory(buffer, size, DirectX::DDS_FLAGS_NONE, nullptr, image);
    if (res != S_OK) res = DirectX::LoadFromHDRMemory(buffer, size, nullptr, image);
    if (res != S_OK) res = DirectX::LoadFromTGAMemory(buffer, size, DirectX::TGA_FLAGS_NONE, nullptr, image);
    if (res != S_OK) res = DirectX::LoadFromWICMemory(buffer, size, DirectX::WIC_FLAGS_NONE, nullptr, image);

    metadata.format = res == S_OK ? GetFormatFromDXGI(image.GetMetadata().format, formatSpace) : Texture_unknown;

    // TODO: Sampler parameters when using mipmaps

    if (metadata.format != Texture_unknown)
    {
        const DirectX::TexMetadata& srcMetadata = image.GetMetadata();

        metadata.width = uint(srcMetadata.width);
        metadata.height = uint(srcMetadata.height);
        metadata.depth = uint(srcMetadata.depth);
        metadata.arraySize = uint(srcMetadata.arraySize);

        switch (srcMetadata.dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE2D:
        {
            bool compressed = IsCompressed(metadata.format);
            uint glInternal = GetGLInternalFormat(metadata.format, colorSpace.value_or(formatSpace));
            uint glFormat = GetGLFormat(metadata.format);
            uint glType = GetGLType(metadata.format);

            if (srcMetadata.IsCubemap()) 
            {
                metadata.texType = TextureType_Cube;
                TextureCube* texture = new TextureCube;

                for (uint32_t i = 0; i < srcMetadata.mipLevels; ++i)
                {
                    for (uint32_t j = 0; j < srcMetadata.arraySize; ++j)
                    {
                        const DirectX::Image* mip = image.GetImage(i, j, 0);

                        if (compressed)
                        {
                            texture->SetCompressedData(j, i, uint(mip->width), uint(mip->height), glInternal, uint(mip->rowPitch * mip->height), mip->pixels);
                        }
                        else
                        {
                            texture->SetData(j, i, uint(mip->width), uint(mip->height), glInternal, glFormat, glType, mip->pixels);
                        }
                    }
                }

                if (srcMetadata.mipLevels > 1)
                {
                    texture->SetMinMaxFiler(GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR);
                    texture->SetTextureLodLevels(0, uint(srcMetadata.mipLevels - 1));
                }

                result = texture;
            }
            else 
            {
                metadata.texType = TextureType_2D;
                Texture2D* texture = new Texture2D(GL_TEXTURE_2D);

                for (uint32_t i = 0; i < srcMetadata.mipLevels; ++i)
                {
                    const DirectX::Image *mip = image.GetImage(i, 0, 0);

                    if (compressed)
                    {
                        texture->SetCompressedData(uint(mip->width), uint(mip->height), i, glInternal, uint(mip->rowPitch * mip->height), mip->pixels);
                    }
                    else
                    {
                        texture->SetData(uint(mip->width), uint(mip->height), i, glInternal, glFormat, glType, mip->pixels);
                    }
                }

                if (srcMetadata.mipLevels > 1)
                {
                    texture->SetMinMaxFiler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                    texture->SetTextureLodLevels(0, uint(srcMetadata.mipLevels-1));
                }

                result = texture;
            }
            break;
        }
        case DirectX::TEX_DIMENSION_TEXTURE3D:
            {
                metadata.texType = TextureType_3D;
                assert(false && "Unsupported texture");
            }
            break;
        default:
            assert(false && "Unsupported texture dimension");
            break;
        }

        metadata.memSize = uint(image.GetPixelsSize());
    }

    return result;
}

bool ResourceTexture::LoadCheckers()
{
    user_name = "*Checkers Texture Preset*";
    file = "*Checkers Texture Preset*";
    exported_file = "*Checkers Texture Preset*";

    // http://www.glprogramming.com/red/chapter09.html

    GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

    for (int i = 0; i < CHECKERS_HEIGHT; i++) {
        for (int j = 0; j < CHECKERS_WIDTH; j++) {
            int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
            checkImage[i][j][3] = (GLubyte)255;
        }
    }

    metadata.width = CHECKERS_WIDTH;
    metadata.height = CHECKERS_HEIGHT;
    metadata.depth = 4;
    metadata.mipCount = 1;
    metadata.format = Texture_rgba;

    glTexture = std::make_unique<Texture2D>(CHECKERS_WIDTH, CHECKERS_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, checkImage, false);

    return true;
}

bool ResourceTexture::LoadFallback(ResourceTexture* resource, const float3& color)
{
    resource->file = "*Fallback Texture Preset*";
    resource->exported_file = "*Fallback Texture Preset*";

    GLubyte fallbackImage[3] = { GLubyte(255 * color.x), GLubyte(255 * color.y), GLubyte(255 * color.z) };

    uint ImageName = 0;

    resource->metadata.width = 1;
    resource->metadata.height = 1;
    resource->metadata.depth = 3;
    resource->metadata.mipCount = 1; 
    resource->metadata.format = Texture_bgr;

    resource->glTexture = std::make_unique<Texture2D>(1, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, fallbackImage, false);

    return true;
}

bool ResourceTexture::LoadRedImage(ResourceTexture* resource, uint width, uint height)
{
    if (!resource->glTexture)
    {
        resource->glTexture = std::make_unique<Texture2D>(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, false);
        resource->metadata.texType = TextureType_2D;
        resource->metadata.width = width;
        resource->metadata.height = height;
        resource->metadata.depth = 1;
        resource->metadata.arraySize = 1;
        resource->metadata.format = Texture_rgba;
        resource->colorSpace = ColorSpace_linear;
    }

    std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/redImage.glsl");
    std::unique_ptr<Program> program;

    bool ok = shader->Compiled();

    if (ok)
    {
        program = std::make_unique<Program>(shader.get());
        ok = program->Linked();
    }

    if (ok)
    {
        program->Use();
        program->BindUniform(REDIMAGE_WIDHT_LOCATION, int(width));
        program->BindUniform(REDIMAGE_HEIGHT_LOCATION, int(height));

        resource->glTexture->BindImage(REDIMAGE_IMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);

        uint numGroupsX = (width + (REDIMAGE_GROUP_WIDTH - 1)) / REDIMAGE_GROUP_WIDTH;
        uint numGroupsY = (height + (REDIMAGE_GROUP_HEIGHT - 1)) / REDIMAGE_GROUP_HEIGHT;
        glDispatchCompute(numGroupsX, numGroupsY, 1);
        //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    return ok;
}


