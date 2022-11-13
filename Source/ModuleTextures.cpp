#include "Globals.h"
#include "Application.h"
#include "OpenGL.h"
#include "OGL.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "IL/il.h"
#include "IL/ilut.h"
#include "ResourceTexture.h"
#include "ModuleResources.h"

#include "DirectXTex/DirectXTex.h"

#include "Leaks.h"

#include <assert.h>
#include <algorithm>
#include <set>
#include <unordered_set>

#pragma comment( lib, "Devil/lib/x64/Release/DevIL.lib" )
#pragma comment( lib, "Devil/lib/x64/Release/ILU.lib" )
#pragma comment( lib, "Devil/lib/x64/Release/ILUT.lib" )

#pragma comment( lib, "DirectXTex.lib" )

using namespace std;

#ifdef max 
#undef max 
#endif

namespace
{

    bool LoadImage(const void* buffer, uint size, ILuint& image, bool& compressed, uint flip_origin = IL_ORIGIN_UPPER_LEFT)
    {
        ilGenImages(1, &image);
        ilBindImage(image);

        bool ok = compressed = ilLoadL(IL_DDS, (const void*)buffer, size);
        if (!compressed)
        {
            ok = ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size);;
        }

        if (ok)
        {
            GLuint textureId = 0;

            ILinfo ImageInfo;
            iluGetImageInfo(&ImageInfo);
            if (ImageInfo.Origin == flip_origin)
            {
                iluFlipImage();
            }

            return true;
        }

        return false;
    }

}

ModuleTextures::ModuleTextures(bool start_enabled) : Module("Textures", start_enabled)
{
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
}

// Destructor
ModuleTextures::~ModuleTextures()
{
	ilShutDown();
}

// Called before render is available
bool ModuleTextures::Init(Config* config)
{
	LOG("Init Image library using DevIL lib version %d", IL_VERSION);

	return true;
}

// Called before quitting
bool ModuleTextures::CleanUp()
{
	LOG("Freeing textures and Image library");

	return true;
}

bool ModuleTextures::ImportCube(const std::string files [], std::string& output_file)
{
    bool ret = true;
    void* output_buffers[6];
    uint  output_sizes[6];
    uint total_size  = 0;

    for (uint i = 0; ret && i < 6; ++i)
    {
        char* buffer = nullptr;
        uint size = App->fs->Load((char*) files[i].c_str(), &buffer);

        ret = buffer != nullptr;
        ret = ret && Import(buffer, size, 0, output_buffers[i], output_sizes[i]);

        total_size += output_sizes[i];

        RELEASE_ARRAY(buffer);
    }

    char* total_buffer = nullptr;

    if (ret)
    {
        total_buffer = (char*)malloc(total_size + sizeof(uint32_t) + sizeof(uint32_t) * 6);
        *(uint32_t*)total_buffer = uint32_t(ResourceTexture::TextureCube);

        uint buffer_pos = sizeof(uint32_t);

        for (uint i = 0; i < 6; ++i)
        {
            *reinterpret_cast<uint32_t*>(&total_buffer[buffer_pos]) = output_sizes[i];
            buffer_pos += sizeof(uint32_t);
            memcpy(&total_buffer[buffer_pos], output_buffers[i], output_sizes[i]);
            buffer_pos += output_sizes[i];

            RELEASE_ARRAY(output_buffers[i]);
        }
    }

    if(ret)
    {
        ret = App->fs->SaveUnique(output_file, total_buffer, total_size, LIBRARY_TEXTURES_FOLDER, "texture", "tex");
        free(total_buffer);
    }

    return ret;
}

// Import new texture from file path
bool ModuleTextures::Import(const char* file, string& output_file, bool toCubemap)
{
	std::string sFile(file);

	char* buffer = nullptr;
	uint size = App->fs->Load(file, &buffer);


    bool ret = Import(buffer, size, output_file, toCubemap);

    if (ret == false)
    {
        LOG("Cannot load texture %s", file);
    }

    return ret;
}

bool ModuleTextures::Import(const void * buffer, uint size, string& output_file, bool toCubemap)
{
    return toCubemap ? ImportToCubemap(buffer, size, output_file) : Import(buffer, size, output_file);
}

bool ModuleTextures::ImportToCubemap(const void* buffer, uint size, string& output_file)
{
	bool ret = buffer  != nullptr;
    uint8_t* output_buffer = nullptr;
    uint output_size    = 0;

    ILuint image;
    bool compressed;
    if(ret && (ret = LoadImage(buffer, size, image, compressed, IL_ORIGIN_UPPER_LEFT)))
    {
        std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(ilGetInteger(IL_IMAGE_WIDTH), 
                ilGetInteger(IL_IMAGE_HEIGHT), 
                GL_RGBA, ilGetInteger(IL_IMAGE_FORMAT), 
                ilGetInteger(IL_IMAGE_TYPE), ilGetData(), false);

        std::unique_ptr<TextureCube> cubeMap(cubeUtils.ConvertToCubemap(texture.get(), 512, 512));

        output_size = sizeof(uint32_t);

        std::vector<float> tmpBuffer;
        std::vector<uint8_t> sideData[6];

        tmpBuffer.resize(512 * 512 * 3);

        auto writeFunc = [](void* context, void* data, int size)
        {
            std::vector<uint8_t>& sideData = *reinterpret_cast<std::vector<uint8_t>*>(context);
            sideData.insert(sideData.end(), reinterpret_cast<uint8_t*>(data), reinterpret_cast<uint8_t*>(data) + size);
        };

        ILuint ImageName;				  
        ilGenImages(1, &ImageName);
        ilBindImage(ImageName);

        for (uint i = 0; i < 6; ++i)
        {
            cubeMap->GetData(i, 0, GL_RGB, GL_FLOAT, &tmpBuffer[0]);

            ilTexImage(512, 512, 1, 3, IL_RGB, IL_FLOAT, &tmpBuffer[0]);
            uint32_t size = ilSaveL(IL_HDR, NULL, 0 ); // Get the size of the data buffer
            if(size > 0) 
            {
                sideData[i].resize(size);

                ret = ilSaveL(IL_HDR, &sideData[i][0], size) > 0;
            }

            output_size += uint(sizeof(uint32_t) + sideData[i].size());
        }

        ilDeleteImage(ImageName);

        output_buffer = (uint8_t*)malloc(output_size);

        *(uint32_t*)output_buffer = uint32_t(ResourceTexture::TextureCube);

        uint buffer_pos = sizeof(uint32_t);

        for(uint i=0; i< 6; ++i)
        {
            *reinterpret_cast<uint32_t*>(&output_buffer[buffer_pos]) = uint(sideData[i].size());
            buffer_pos += sizeof(uint32_t);
            memcpy(&output_buffer[buffer_pos], &sideData[i][0], sideData[i].size());
            
            buffer_pos += uint(sideData[i].size());
        }

        ilDeleteImages(1, &image);
    }

    if(ret)
    {
        ret = App->fs->SaveUnique(output_file, output_buffer, output_size, LIBRARY_TEXTURES_FOLDER, "texture", "tex");

        RELEASE_ARRAY(buffer);
        RELEASE_ARRAY(output_buffer);
    }

	return ret;
}

bool ModuleTextures::Import(const void * buffer, uint size, string& output_file)
{   
	bool ret = buffer != nullptr;

    void* output_buffer = nullptr;
    uint output_size = 0;
    uint header_size = sizeof(uint32_t);

    ret = ret && Import(buffer, size, header_size, output_buffer, output_size);

    if (ret)
    {
        *reinterpret_cast<uint32_t*>(output_buffer) = uint32_t(ResourceTexture::Texture2D);
        ret = App->fs->SaveUnique(output_file, output_buffer, output_size + header_size, LIBRARY_TEXTURES_FOLDER, "texture", "tex");

        RELEASE_ARRAY(buffer);
        RELEASE_ARRAY(output_buffer);
    }

	return ret;

}


bool ModuleTextures::Import(const void* buffer, uint size, uint header_size, void*& output_buffer, uint& output_size)
{
	bool ret = false;

	if (buffer)
	{        
        DirectX::ScratchImage image, fliped, converted;
        DirectX::ScratchImage* result = &image;
        HRESULT res = DirectX::LoadFromDDSMemory(buffer, size, DirectX::DDS_FLAGS_NONE, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromHDRMemory(buffer, size, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromTGAMemory(buffer, size, DirectX::TGA_FLAGS_NONE, nullptr, image);
        if (res != S_OK) res = DirectX::LoadFromWICMemory(buffer, size, DirectX::WIC_FLAGS_NONE, nullptr, image);

        if (res == S_OK)
        {
            if (DirectX::FlipRotate(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::TEX_FR_FLIP_VERTICAL, fliped) == S_OK)
            {
                result = &fliped;
            }
        }

        if (res == S_OK)
        {
            DirectX::Blob blob;            
            ret = DirectX::SaveToDDSMemory(result->GetImages(), result->GetImageCount(), result->GetMetadata(), DirectX::DDS_FLAGS_NONE, blob) == S_OK;
            
            if (ret == S_OK)
            {
                output_size = uint(blob.GetBufferSize());
                output_buffer = new char[blob.GetBufferSize()+header_size];
                memcpy(&reinterpret_cast<char*>(output_buffer)[header_size], blob.GetBufferPointer(), blob.GetBufferSize());
            }
        }
    }

	if (ret == false)
		LOG("Cannot load texture from buffer of size %u", size);

	return ret;
}

// Load new texture from file path
bool ModuleTextures::Load(ResourceTexture* resource)
{

	char* head_buffer = nullptr;
	uint total_size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, resource->GetExportedFile(), &head_buffer);

    bool ret = head_buffer != nullptr && total_size > 0;

	if (ret)
	{
        resource->type = ResourceTexture::Type(*reinterpret_cast<uint32_t*>(head_buffer));
        char* buffer = head_buffer+sizeof(uint32_t);

        if(resource->type == ResourceTexture::Texture2D)
        {
            /*
            DirectX::ScratchImage image;
            HRESULT res = DirectX::LoadFromDDSMemory(buffer, total_size, DirectX::DDS_FLAGS_NONE, nullptr, image);

            if (res == S_OK)
            {
                resource->width = image.GetMetadata().width;
                resource->height = image.GetMetadata().height;
                resource->compressed = true;

                resource->texture = std::make_unique<Texture2D>(resource->width, resource->height,
                    !resource->GetLinear() ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
                    image.GetPixelsSize(), image.GetPixels(), resource->has_mips);
            }
            else*/
            {
                /*
                ILuint image;

                if (LoadImage(buffer, total_size, image, resource->compressed, IL_ORIGIN_UPPER_LEFT))
                {
                    resource->width = ilGetInteger(IL_IMAGE_WIDTH);
                    resource->height = ilGetInteger(IL_IMAGE_HEIGHT);

                    if (resource->compressed)
                    {
                        ILuint compressedSize = ilGetDXTCData(nullptr, 0, IL_DXT5);
                        char* compressedData = new char[compressedSize];
                        ilGetDXTCData(compressedData, compressedSize, IL_DXT5);

                        resource->texture = std::make_unique<Texture2D>(resource->width, resource->height,
                            !resource->GetLinear() ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
                            compressedSize, compressedData, resource->has_mips);

                        delete[] compressedData;
                    }
                    else
                    {
                        resource->texture = std::make_unique<Texture2D>(resource->width, resource->height,
                            !resource->GetLinear() ? GL_SRGB8_ALPHA8 : GL_RGBA, ilGetInteger(IL_IMAGE_FORMAT),
                            ilGetInteger(IL_IMAGE_TYPE), ilGetData(), resource->has_mips);
                    }
                }
                ilDeleteImages(1, &image);*/
            }
        }
        else if(resource->type == ResourceTexture::TextureCube)
        {
            TextureCube* cube = new TextureCube();
            resource->glTexture = std::unique_ptr<TextureCube>(cube);

            for(uint i=0; i<6; ++i)
            {
                uint size = *reinterpret_cast<uint32_t*>(buffer);
                buffer += sizeof(uint32_t);

                ILuint image;
                bool compressed;
                if(LoadImage(buffer, size, image, compressed, IL_ORIGIN_UPPER_LEFT))
                {
                    cube->SetData(i, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
                        resource->GetColorSpace() == ResourceTexture::gamma ? GL_SRGB8_ALPHA8 : GL_RGBA,
                        ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
                }

                ilDeleteImages(1, &image);

                buffer += size;
            }
        }
            
    }

	RELEASE_ARRAY(head_buffer);
	return ret;
}

bool ModuleTextures::LoadToArray(const ResourceTexture* resource, Texture2DArray* texture, uint index)
{
    assert(resource->GetTexType() == ResourceTexture::Texture2D);

    char* head_buffer = nullptr;
    uint total_size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, resource->GetExportedFile(), &head_buffer);

    bool ret = head_buffer != nullptr && total_size > 0;

    if (ret)
    {
        char* buffer = head_buffer + sizeof(uint32_t);

        if (resource->GetTexType() == ResourceTexture::Texture2D)
        {
            DirectX::ScratchImage image;
            HRESULT res = DirectX::LoadFromDDSMemory(buffer, total_size - sizeof(uint32_t), DirectX::DDS_FLAGS_NONE, nullptr, image);

            if (res == S_OK)
            {
                if (image.GetMetadata().format == DXGI_FORMAT_BC3_UNORM)
                {
                    texture->SetCompressedSubData(0, index, resource->GetColorSpace() == ResourceTexture::linear ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, uint(image.GetPixelsSize()), image.GetPixels());
                }
                else
                {
                    texture->SetSubData(0, index, GL_RGBA, GL_UNSIGNED_BYTE, image.GetPixels());
                }
            }
            else
            {
                ILuint image;
                bool compressed = false;

                if (LoadImage(buffer, total_size, image, compressed, IL_ORIGIN_UPPER_LEFT))
                {
                    assert(compressed == resource->IsCompressed());
                    if (resource->IsCompressed())
                    {
                        ILuint compressedSize = ilGetDXTCData(nullptr, 0, IL_DXT5);
                        char* compressedData = new char[compressedSize];
                        ilGetDXTCData(compressedData, compressedSize, IL_DXT5);

                        texture->SetCompressedSubData(0, index, resource->GetColorSpace() == ResourceTexture::linear ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, compressedSize, compressedData);
                        delete[] compressedData;
                    }
                    else
                    {

                        /* Forced to be UNSIGNED_BYTE ??? take into account format when creating*/
                        /* TODO: Create different channels size IL_IMAGE_CHANNELS*/
                        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

                        texture->SetSubData(0, index, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
                    }

                    ilDeleteImages(1, &image);
                }
                else
                {
                    int i = 0;
                    ++i;
                }
            }
        }
    }

    RELEASE_ARRAY(head_buffer);

	return ret;
}


//  Create checkerboard texture  
#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

bool ModuleTextures::LoadCheckers(ResourceTexture * resource)
{
	resource->user_name = "*Checkers Texture Preset*";
	resource->file = "*Checkers Texture Preset*";
	resource->exported_file = "*Checkers Texture Preset*";

	// http://www.glprogramming.com/red/chapter09.html

	GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i&0x8)==0)^(((j&0x8))==0)))*255;
			checkImage[i][j][0] = (GLubyte) c;
			checkImage[i][j][1] = (GLubyte) c;
			checkImage[i][j][2] = (GLubyte) c;
			checkImage[i][j][3] = (GLubyte) 255;
		}
	}

	resource->width = CHECKERS_WIDTH;
	resource->height = CHECKERS_HEIGHT;
	resource->depth = 4;
	resource->mipMaps = false;
	resource->format = ResourceTexture::rgba;
    
    resource->glTexture = std::make_unique<Texture2D>(CHECKERS_WIDTH, CHECKERS_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, checkImage, false);

	return true;
}

bool ModuleTextures::LoadFallback(ResourceTexture* resource, const float3& color)
{
	resource->file = "*Fallback Texture Preset*";
	resource->exported_file = "*Fallback Texture Preset*";

	GLubyte fallbackImage[3] = { GLubyte(255*color.x), GLubyte(255*color.y), GLubyte(255*color.z) };

	uint ImageName = 0;

	resource->width = 1;
	resource->height = 1;
	resource->depth = 3;
	resource->mipMaps = false;
	resource->format = ResourceTexture::bgr;

    resource->glTexture = std::make_unique<Texture2D>(1, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, fallbackImage, false);

	return true;
}


bool ModuleTextures::LoadCube(ResourceTexture* resource, const char* files [], const char* path)
{
    std::string sPath(path);

    TextureCube* cube = new TextureCube();
    resource->glTexture = std::unique_ptr<TextureCube>(cube);
    resource->textype = ResourceTexture::TextureCube;

    bool ret = true;

    for (uint i = 0; ret && i < 6; ++i)
    {
        std::string sFile(files[i]);

        char* buffer = nullptr;
        uint size = App->fs->Load((char*) (sPath + sFile).c_str(), &buffer);

        ret = buffer != nullptr;

        ILuint image;
        bool compressed; 
        ret = ret && LoadImage(buffer, size, image, compressed, IL_ORIGIN_LOWER_LEFT);

        if(ret)
        {
            cube->SetData(i, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
                    resource->GetColorSpace() == ResourceTexture::gamma ? GL_SRGB8_ALPHA8 : GL_RGBA,
                    ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
        }

        ilDeleteImages(1, &image);

        RELEASE_ARRAY(buffer);
    }

    return ret;
}
