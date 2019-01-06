#include "ResourceTexture.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "Config.h"

#include "OpenGL.h"

// ---------------------------------------------------------
ResourceTexture::ResourceTexture(UID uid) : Resource(uid, Resource::Type::texture)
{}

// ---------------------------------------------------------
ResourceTexture::~ResourceTexture()
{}

// ---------------------------------------------------------
const char * ResourceTexture::GetFormatStr() const
{
	static const char* formats[] = { 
		"color index", "rgb", "rgba", "bgr", "bgra", "luminance", "unknown" };

	return formats[format];
}

// ---------------------------------------------------------
bool ResourceTexture::LoadInMemory()
{
	return App->tex->Load(this);
}

// ---------------------------------------------------------
void ResourceTexture::ReleaseFromMemory() 
{
    if(gpu_id != 0)
    { 
        glDeleteTextures(1, &gpu_id);
        gpu_id = 0;
    }
}

// ---------------------------------------------------------
void ResourceTexture::Save(Config & config) const
{
	Resource::Save(config);

	config.AddInt("Format", format);
	config.AddBool("Mipmaps", has_mips);
	config.AddBool("Linear", linear);
}

// ---------------------------------------------------------
void ResourceTexture::Load(const Config & config)
{
	Resource::Load(config);

    format = (Format) config.GetInt("Format", unknown);
    has_mips   = config.GetBool("Mipmaps", false);
    linear     = config.GetBool("Linear", true);

    std::string extension;
    App->fs->SplitFilePath(exported_file.c_str(), nullptr, nullptr, &extension);

    compressed = _stricmp(extension.c_str(), "dds") == 0;
}

// ---------------------------------------------------------
void ResourceTexture::EnableMips(bool enable)
{
    if(has_mips != enable)
    {
        has_mips = enable;

        if(gpu_id != 0)
        {
            if(has_mips)
            {
                glBindTexture(GL_TEXTURE_2D, gpu_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, gpu_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
}

// ---------------------------------------------------------
void ResourceTexture::SetLinear(bool l)
{
    if(l != linear)
    {
        linear = l;

        if(loaded > 0)
        {
            ReleaseFromMemory();
            LoadInMemory();
        }
    }
}

