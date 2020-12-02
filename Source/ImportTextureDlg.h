#ifndef __IMPORT_TEXTURE_DLG_H__
#define __IMPORT_TEXTURE_DLG_H__

#include <string>

class ImportTexturesDlg
{
    std::string file;
    bool        compressed = true;
    bool        mipmaps    = true;
    bool        srgb       = true;
    bool        selection  = false;
    
public:

    void Open           (const std::string& _file);
    void Display        ();
    void ClearSelection ();

    bool               HasSelection () const { return selection; }
    const std::string& GetFile      () const { return file; }
    bool               GetCompressed() const { return compressed; }
    bool               GetMipmaps   () const { return mipmaps; }
    bool               GetSRGB      () const { return srgb; }
};

#endif /* __IMPORT_TEXTURE_DLG_H__ */
