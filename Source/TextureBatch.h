#pragma once

#include "OGL.h"
#include <vector>

class ResourceTexture;
class ResourceMaterial;

class TextureBatch
{
public:
    struct Handle
    {
        int index = 0;
        float layer = 0.0f;
    };

private:
    struct TexData
    {
        const ResourceTexture* texture = nullptr;
        uint refCount = 0;
    };
    struct TexArrayInfo
    {
        std::unique_ptr<Texture2DArray> textureArray;
        std::vector<TexData> textures;
    };

    typedef std::unique_ptr<TexArrayInfo[]> TexArrayPtr;

    TexArrayPtr  textures;
    uint         maxUnits = 0;
    bool         texturesDirty = false;
public:

    TextureBatch();

    bool CanAdd(const ResourceMaterial* material) const;
    bool CanAdd(const ResourceTexture* texture) const;

    bool Add(const ResourceMaterial* material);
    bool Add(const ResourceTexture* texture);

    void Remove(const ResourceMaterial* material);
    void Remove(const ResourceTexture* texture);

    bool GetHandle(const ResourceTexture* texture, Handle& handle) const;

    void Bind();
    void GenerateTextures();

private:
    static uint GetMaxLayers();
    static uint GetFormat(const ResourceTexture* texture);
};