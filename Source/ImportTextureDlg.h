#ifndef __IMPORT_TEXTURE_DLG_H__
#define __IMPORT_TEXTURE_DLG_H__

#include <string>

class ImportTexturesDlg
{
    std::string file;
    std::string open_name;
    bool        mipmaps    = true;

    bool        toCubemap  = false;
    bool        selection  = false;
    bool        open_flag  = false;
    
public:

    ImportTexturesDlg();

    void Open           (const std::string& _file);
    void Display        ();
    void ClearSelection ();

    bool               HasSelection () const { return selection; }
    const std::string& GetFile      () const { return file; }
    bool               GetMipmaps   () const { return mipmaps; }
    bool               GetToCubemap () const { return toCubemap; }
};

#endif /* __IMPORT_TEXTURE_DLG_H__ */
