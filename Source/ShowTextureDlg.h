#ifndef __SHOW_TEXTURE_DLG_H__
#define __SHOW_TEXTURE_DLG_H__

#include <string>

#include "Math.h"

class Texture2D;

class ShowTextureDlg
{
    std::string  open_name;
    bool         open_flag    = false;
    bool         show_uvs     = false;
    bool         show_texture = false;
    float4       uv_color     = float4(1.0f, 1.0f, 1.0f, 1.0f);
    uint         uv_set       = 0;
    float        zoom         = 100.0;
    Texture2D*   texture      = 0;
    uint         width        = 0;
    uint         height       = 0;
public:

    ShowTextureDlg();

    void Open           (Texture2D* _texture, uint _width, uint _height);
    void Display        ();
    void Clear          ();
};

#endif // __SHOW_TEXTURE_DLG_H__


