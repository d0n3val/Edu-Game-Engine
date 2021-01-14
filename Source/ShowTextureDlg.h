#ifndef __SHOW_TEXTURE_DLG_H__
#define __SHOW_TEXTURE_DLG_H__

#include <string>

#include "Math.h"

class Texture2D;
class ResourceMesh;
class Framebuffer;

class ShowTextureDlg
{
    typedef std::unique_ptr<Framebuffer> FramebufferPtr;
    typedef std::unique_ptr<Texture2D> TexturePtr;

    std::string         open_name;
    bool                open_flag    = false;
    bool                show_uvs     = false;
    bool                show_texture = true;
    float4              uv_color     = float4(1.0f, 1.0f, 1.0f, 1.0f);
    uint                uv_set       = 0;
    float               zoom         = 100.0;
    uint                width        = 0;
    uint                height       = 0;
    const ResourceMesh* mesh         = nullptr;
    Texture2D*          source       = 0;
    TexturePtr          target       = 0;
    FramebufferPtr      source_fb;
    FramebufferPtr      target_fb;

public:

    ShowTextureDlg();

    void Open           (const ResourceMesh* _mesh, Texture2D* _texture, uint _width, uint _height);
    void Display        ();
    void Clear          ();

private:

    void GeneratePreview();
    void GenerateSourceFB();
    void GenerateTargetFB();
    void DrawPreviewUVs();
};

#endif // __SHOW_TEXTURE_DLG_H__


