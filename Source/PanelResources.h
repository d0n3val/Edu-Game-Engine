#ifndef __PANELRESOURCES_H__
#define __PANELRESOURCES_H__

#include "Panel.h"
#include "Resource.h"
#include "Imgui/imgui.h"
#include "imgui-filebrowser/imfilebrowser.h"
#include "ImportAnimationDlg.h"
#include "ImportTextureDlg.h"
#include "ImportCubemapDlg.h"
#include "ImportModelDlg.h"

#include <vector>
#include <set>

class PanelResources : public Panel
{
public:
	PanelResources();
	virtual ~PanelResources();

	void Draw() override;


private:
	void DrawResourceType(Resource::Type type, void (PanelResources::*popup)(Resource::Type));

    void DrawMeshPopup(Resource::Type type);
    void DrawResourcePopup(Resource::Type type);
    void DrawPlaneProperties();
    void DrawCylinderProperties();
    void DrawSphereProperties();

    void DrawAnimationProperties();
    void ManageSelection(const std::vector<const Resource*>& resources, uint current, bool append, bool multiple, uint pivot, bool popup);
    void ImportResource(const std::string& file);

private:

    ImportTexturesDlg   textures_dlg;
    ImportAnimationDlg  animation_dlg;
    ImportCubemapDlg    cubemap_dlg;
    ImportModelDlg      model_dlg;

	bool waiting_to_load_file      = false;
    Resource::Type waiting_to_load = Resource::unknown;
    std::set<UID> selection;
    Resource::Type multiple_select_type = Resource::unknown;
    uint multiple_select_pivot          = 0;
    ImGui::FileBrowser fileDialog;

};

#endif  // __PANELRESOURCES_H__
