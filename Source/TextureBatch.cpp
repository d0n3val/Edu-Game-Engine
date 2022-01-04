#include "Globals.h"
#include "TextureBatch.h"

#include "Application.h"
#include "ModuleTextures.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"

#include "OpenGL.h"

#include "Leaks.h"

#include <algorithm>

#ifdef min
#undef min
#endif 

TextureBatch::TextureBatch()
{
}

bool TextureBatch::CanAdd(const ResourceMaterial *material) const
{
    bool ok = true;

    for(int i=0; ok && i< TextureCount; ++i)
    {
        const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));
        ok = texture == nullptr || CanAdd(texture);
    }

    return ok;
}

bool TextureBatch::CanAdd(const ResourceTexture *texture) const
{
    uint i = 0;
    for (; i < MAX_TEXTURE_ARRAY_COUNT; ++i)
    {
        const TexArrayInfo& info = textures[i];

        if(info.textures.empty())
        {
            break;
        }

        const ResourceTexture* res = info.textures.front().texture;
        if(res == texture)
        {
            break;
        }

        if (res->GetWidth() == texture->GetWidth() && res->GetHeight() == texture->GetHeight() && 
            res->GetCompressed() == texture->GetCompressed() && res->GetLinear() == texture->GetLinear() &&
            res->HasMips() == texture->HasMips() && info.textures.size() < GetMaxLayers())
        {
            break;
        }
    }

    return i < MAX_TEXTURE_ARRAY_COUNT;
}

bool TextureBatch::Add(const ResourceMaterial *material)
{
    bool ok = true;

    for(int i=0; ok && i< TextureCount; ++i)
    {
        const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));
        ok = texture == nullptr || Add(texture);
    }

    return ok;
}

bool TextureBatch::Add(const ResourceTexture *texture)
{
    // check already in
    for (uint i = 0; i < MAX_TEXTURE_ARRAY_COUNT; ++i)
    {
        TexArrayInfo& info = textures[i];
        if(!info.textures.empty())
        {
            for(TexData& texData : info.textures)
            {
                if(texData.texture == texture)
                {
                    ++texData.refCount;
                    return true;
                }
            }
        }
    }

    // check for any place
    uint index = 0;
    for (; index < MAX_TEXTURE_ARRAY_COUNT; ++index)
    {
        TexArrayInfo& info = textures[index];

        if(info.textures.empty())
        {
            break;
        }

        TexData& texData = info.textures.front();

        if (texData.texture->GetWidth() == texture->GetWidth() && texData.texture->GetHeight() == texture->GetHeight() && 
            texData.texture->GetCompressed() == texture->GetCompressed() && texData.texture->GetLinear() == texture->GetLinear() &&
            texData.texture->HasMips() == texture->HasMips() && info.textures.size() < GetMaxLayers())
        {
            break;
        }
    }

    if(index < MAX_TEXTURE_ARRAY_COUNT) // add new Texture Array
    {
        TexData texData = { texture, 1 };
        textures[index].textures.push_back(texData);
        texturesDirty = true;

        return true;
    }

    return false;
}

void TextureBatch::Remove(const ResourceMaterial *material)
{
    for(int i=0; i< TextureCount; ++i)
    {
        const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));
        if(texture != nullptr) 
        {
            Remove(texture);
        }
    }
}

void TextureBatch::Remove(const ResourceTexture *texture)
{
    for (uint i = 0; i < MAX_TEXTURE_ARRAY_COUNT; ++i)
    {
        TexArrayInfo& info = textures[i];
        if(!info.textures.empty())
        {
            for(auto it = info.textures.begin(); it != info.textures.end(); ++it)
            {
                if(it->texture == texture)
                {
                    if((--it->refCount) == 0)
                    {
                        info.textures.erase(it);
                        return;
                    }
                }
            }
        }
    }
}

void TextureBatch::Bind()
{
    if(texturesDirty)
    {
        GenerateTextures();
    }

    for(int i = 0; i< MAX_TEXTURE_ARRAY_COUNT; ++i)
    {
        const TexArrayInfo& info = textures[i];
        if(info.textures.empty())
        {
            break;
        }
        else
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i].textureArray->Id());
        }
    }
}

uint TextureBatch::GetMaxLayers()
{
    static GLint max_layers = 0;

    if(max_layers == 0)
    {
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);
    }

    return max_layers;
}

void TextureBatch::GenerateTextures()
{
    for(int i = 0; i< MAX_TEXTURE_ARRAY_COUNT; ++i)
    {
        TexArrayInfo& info = textures[i];
        if(info.textures.empty())
        {
            break;
        }
        else
        {
            const ResourceTexture* front = info.textures.front().texture;

            uint minSize = std::min(front->GetWidth(), front->GetHeight());
            uint levels = uint(log(float(minSize))/log(2.0f))-3;

            info.textureArray = std::make_unique<Texture2DArray>(levels, front->GetWidth(), front->GetHeight(), uint(info.textures.size()), GetFormat(front));

            for(uint j=0; j < info.textures.size(); ++j)
            {
                App->tex->LoadToArray(info.textures[j].texture, info.textureArray.get(), j);
            }

            if(front->HasMips())
            {
                info.textureArray->GenerateMipmaps();
            }
        }
    }

    texturesDirty = false;
}

bool TextureBatch::GetHandle(const ResourceTexture *texture, Handle &handle) const
{
    assert(texturesDirty == false);

    for (uint index = 0; index < MAX_TEXTURE_ARRAY_COUNT; ++index)
    {
        const TexArrayInfo& info = textures[index];

        if(!info.textures.empty())
        {
            const ResourceTexture *front = info.textures.front().texture;

            if (front->GetWidth() == texture->GetWidth() && front->GetHeight() == texture->GetHeight() &&
                front->GetCompressed() == texture->GetCompressed() && front->GetLinear() == texture->GetLinear() &&
                front->HasMips() == texture->HasMips() && info.textures.size() < GetMaxLayers())
            {
                for (uint layer = 0; layer < uint(info.textures.size()); ++layer)
                {
                    if (info.textures[layer].texture == texture)
                    {
                        handle.index = index;
                        handle.layer = float(layer);
                        return true;
                    }
                }
            }
        }
    }

    handle.index = 0;
    handle.layer = 0.0f;

    return false;
}

uint TextureBatch::GetFormat(const ResourceTexture *texture) 
{
    if (texture->GetCompressed())
    {
        return texture->GetLinear() ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
    }

    return texture->GetLinear() ? GL_RGBA8 : GL_SRGB8_ALPHA8;
}