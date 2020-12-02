#ifndef __IMPORT_CUBEMAP_DLG_H__
#define __IMPORT_CUBEMAP_DLG_H__

#include "imgui/imgui.h"
#include "imgui-filebrowser/imfilebrowser.h"
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
    bool                open_flag    = false;
    bool                in_file_flag = false;

public:

    void Open           ();
    void Display        ();
    void ClearSelection ();

    bool HasSelection() const { return selection; }
    const std::string& GetFile(Side aSide) const { return files[aSide]; }

};

#endif /* __IMPORT_CUBEMAP_DLG_H__ */
