#include "ResourceTexture.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "Config.h"

#include "DirectXTex/DirectXTex.h"

#include "OpenGL.h"

#include "Leaks.h"

#include <memory>
#include <assert.h>

namespace
{
    DXGI_FORMAT GetDXGIFromCompressType(ResourceTexture::CompressType type, bool opaqueAlpha, DirectX::TEX_COMPRESS_FLAGS& flags)
    {
        flags = DirectX::TEX_COMPRESS_DEFAULT;
        switch (type)
        {
        case ResourceTexture::Compress_Colour:
            if (opaqueAlpha) return DXGI_FORMAT_BC1_UNORM;
            return DXGI_FORMAT_BC3_UNORM;
        case ResourceTexture::Compress_Grayscale:
            return DXGI_FORMAT_BC4_UNORM;
        case ResourceTexture::Compress_Normals:
            return DXGI_FORMAT_BC5_UNORM;
        case ResourceTexture::Compress_HDR:
            return DXGI_FORMAT_BC6H_SF16;
        case ResourceTexture::Compress_Colour_HQ_Fast:
            flags = DirectX::TEX_COMPRESS_BC7_QUICK;
        case ResourceTexture::Compress_Colour_HQ:
            return DXGI_FORMAT_BC7_UNORM;
        }

        return DXGI_FORMAT_BC1_UNORM;
    }

    ResourceTexture::Format GetFormatFromDXGI(DXGI_FORMAT format, ResourceTexture::ColorSpace& colorSpace)
    {
        colorSpace = ResourceTexture::linear;

        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            colorSpace = ResourceTexture::gamma;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return ResourceTexture::rgba;
            break;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            colorSpace = ResourceTexture::gamma;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return ResourceTexture::bgra;
            break;
        case DXGI_FORMAT_B5G6R5_UNORM:
            return ResourceTexture::bgr;
            break;
        case DXGI_FORMAT_R8_UNORM:
            return ResourceTexture::red;
            break;
        case DXGI_FORMAT_BC1_UNORM:
            return ResourceTexture::bc1;
            break;
        case DXGI_FORMAT_BC3_UNORM:
            return ResourceTexture::bc3;
            break;
        case DXGI_FORMAT_BC4_UNORM:
            return ResourceTexture::bc4;
            break;
        case DXGI_FORMAT_BC5_UNORM:
            return ResourceTexture::bc5;
            break;
        case DXGI_FORMAT_BC6H_SF16:
            return ResourceTexture::bc6s;
            break;
        case DXGI_FORMAT_BC6H_UF16:
            return ResourceTexture::bc6u;
            break;
        case DXGI_FORMAT_BC7_UNORM:
            return ResourceTexture::bc7;
        }

        return ResourceTexture::unknown;
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
	static const char* formats[] = { "RGBA", "BGRA", "BGR", "RED", "BC1", "BC3", "BC4", "BC5", "BC6s", "BC6u", "BC7"};

	return formats[format];
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

        DirectX::ScratchImage image;
        HRESULT res = DirectX::LoadFromDDSMemory(buffer, total_size-sizeof(uint32_t), DirectX::DDS_FLAGS_NONE, nullptr, image);       

        if (res == S_OK)
        {
            const DirectX::TexMetadata& metadata = image.GetMetadata();
            width  = metadata.width;
            height = metadata.height;
            depth  = metadata.depth;
            arraySize = metadata.arraySize;
            format = GetFormatFromDXGI(metadata.format, formatColorSpace);

            switch(metadata.dimension) 
            {
                case DirectX::TEX_DIMENSION_TEXTURE2D:
                    textype = Texture2D;
                    if (IsCompressed())
                    {
                        glTexture = std::make_unique<::Texture2D>(width, height, GetGLInternalFormat(), uint(image.GetPixelsSize()), image.GetPixels(), mipMaps);
                    }
                    else
                    {
                        glTexture = std::make_unique<::Texture2D>(width, height, GetGLInternalFormat(), GetGLFormat(), GetGLType(), image.GetPixels(), mipMaps);
                    }
                    break;
                case DirectX::TEX_DIMENSION_TEXTURE3D:
                    if(metadata.IsCubemap()) 
                    {
                        textype = TextureCube;
                        //glTexture = std::make_unique<::TextureCube>(resource->width, resource->height, 
                          //                                      GetGLInternalFormat(), GetGLType(),image.GetPixelsSize(), image.GetPixels(), resource->has_mips);
                    }
                    else
                    {
                        assert(false && "Unsupported texture");
                    }
                    break;
                default:
                    assert(false && "Unsupported texture dimension");
                    break;
            }

            memSize = image.GetPixelsSize();
        }
    }

    RELEASE_ARRAY(head_buffer);

    return ret;
}

void ResourceTexture::GenerateMipmaps(bool generate)
{
    mipMaps = generate;

    if(glTexture)
        glTexture->GenerateMipmaps(0, generate ? 1000 : 1);
}

// ---------------------------------------------------------
uint ResourceTexture::GetGLInternalFormat() const 
{
    static uint gl_internal[] = { GL_RGBA8, GL_RGBA8, GL_RGB8, GL_R8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM };
    static uint gl_internal_gamma[] = { GL_SRGB8_ALPHA8, GL_SRGB8_ALPHA8 , GL_SRGB8, GL_R8, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_RG_RGTC2 , GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT , GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM , GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM };

    return GetColorSpace() == linear ? gl_internal[uint(format)] : gl_internal_gamma[uint(format)];
}

// ---------------------------------------------------------
uint ResourceTexture::GetGLFormat() const 
{
    static uint gl_format[] = { GL_RGBA, GL_BGRA, GL_BGR, GL_RED };
    assert(uint(format) < 4);

    return gl_format[uint(format)];
}


// ---------------------------------------------------------
uint ResourceTexture::GetGLType() const 
{
    static uint gl_type[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE };
    assert(uint(format) < 4);

    return gl_type[uint(format)];
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
    config.AddBool("Mipmaps", mipMaps);
}

// ---------------------------------------------------------
void ResourceTexture::Load(const Config & config)
{
	Resource::Load(config);

    if(config.HasInt("ColorSpace"))
    {
        colorSpace = (ColorSpace)config.GetInt("ColorSpace", formatColorSpace);
    }

    mipMaps = config.GetBool("Mipmaps", mipMaps);
}

bool ResourceTexture::IsCompressed() const
{
    return format == bc1 ||
           format == bc3 ||
           format == bc4 ||
           format == bc5 ||
           format == bc6s ||
           format == bc6u ||
           format == bc7;
}

bool ResourceTexture::LoadToArray(Texture2DArray* texArray, uint index) const
{
    assert(GetTexType() == ResourceTexture::Texture2D);

	char* head_buffer = nullptr;
	uint total_size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, GetExportedFile(), &head_buffer);

    bool ret = head_buffer != nullptr && total_size > 0;

	if (ret)
	{
        char *buffer = head_buffer + sizeof(uint32_t);

        DirectX::ScratchImage image;
        HRESULT res = DirectX::LoadFromDDSMemory(buffer, total_size-sizeof(uint32_t), DirectX::DDS_FLAGS_NONE, nullptr, image);

        if (res != S_OK) 
        {
            res = DirectX::LoadFromTGAMemory(buffer, total_size - sizeof(uint32_t), nullptr, image);
        }

        ret = res == S_OK;

        if (ret)
        {
            if (IsCompressed())
            {
                texArray->SetCompressedSubData(0, index, GetGLInternalFormat(), uint(image.GetPixelsSize()), image.GetPixels());
            }
            else
            {
                texArray->SetSubData(0, index, GetGLFormat(), GetGLType(), image.GetPixels());
            }
        }
    }

    RELEASE_ARRAY(head_buffer);

    return ret;
}

void ResourceTexture::Compress(CompressType type)
{
    assert(IsCompressed() == false);
    char* head_buffer = nullptr;
    uint total_size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, GetExportedFile(), &head_buffer);

    if (head_buffer != nullptr && total_size > 0)
    {
        char* buffer = head_buffer + sizeof(uint32_t);

        DirectX::ScratchImage image, compressed;
        HRESULT res = DirectX::LoadFromDDSMemory(buffer, total_size - sizeof(uint32_t), DirectX::DDS_FLAGS_NONE, nullptr, image);
        if (res == S_OK)
        {
            DirectX::TEX_COMPRESS_FLAGS flags;
            DXGI_FORMAT format = GetDXGIFromCompressType(type, image.IsAlphaAllOpaque(),flags);
            res = DirectX::Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), format, flags, 0.2f, compressed);
        }

        if (res == S_OK)
        {
            DirectX::Blob blob;
            res = DirectX::SaveToDDSMemory(compressed.GetImages(), compressed.GetImageCount(), compressed.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob);

            if (res == S_OK)
            {
                uint header_size = sizeof(uint32_t);
                uint output_size = uint(blob.GetBufferSize());
                char* output_buffer = new char[blob.GetBufferSize() + header_size];
                memcpy(&reinterpret_cast<char*>(output_buffer)[header_size], blob.GetBufferPointer(), blob.GetBufferSize());
                *reinterpret_cast<uint32_t*>(output_buffer) = uint32_t(ResourceTexture::Texture2D);

                if (App->fs->Save(LIBRARY_TEXTURES_FOLDER, GetExportedFile(), output_buffer, output_size + header_size))
                {
                    ReleaseFromMemory();
                    LoadInMemory();
                }

                RELEASE_ARRAY(output_buffer);
            }
        }
    }

    RELEASE_ARRAY(head_buffer);
}

