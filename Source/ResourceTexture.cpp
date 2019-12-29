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
    texture.reset(nullptr);
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

        if(texture->Id())
        {
            if(has_mips)
            {
                texture->SetMinMaxFiler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                texture->GenerateMipmaps(0, 1000);
            }
            else
            {
                texture->SetMinMaxFiler(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
                texture->GenerateMipmaps(0, 1000);
            }
        }
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

