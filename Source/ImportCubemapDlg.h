#ifndef __IMPORT_CUBEMAP_DLG_H__
#define __IMPORT_CUBEMAP_DLG_H__

#include "imgui/imgui.h"
#include "imgui-filebrowser/imfilebrowser.h"
#include "ImportTextureDlg.h"
#include <string>

class ImportCubemapDlg
{
public:
    enum Side { Front = 0, Back, Left, Right, Top, Bottom, SideCount };

private:

    std::string         files[SideCount];
    bool                selection = false;
    uint                open_index = 0;
    ImGui::FileBrowser  fileDialog;
    ImportTexturesDlg   textureDlg;
    bool                open_flag    = false;

public:

    void Open           ();
    void Display        ();
    void ClearSelection ();

    bool               HasSelection () const { return selection; }
    const std::string* GetFiles     () const { return files; }
    bool               GetCompressed() const { return textureDlg.GetCompressed(); }
    bool               GetMipmaps   () const { return textureDlg.GetMipmaps(); }
    bool               GetSRGB      () const { return textureDlg.GetSRGB(); }

};

#endif /* __IMPORT_CUBEMAP_DLG_H__ */
