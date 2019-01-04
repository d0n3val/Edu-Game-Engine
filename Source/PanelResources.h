#ifndef __PANELRESOURCES_H__
#define __PANELRESOURCES_H__

#include "Panel.h"
#include "Resource.h"

class PanelResources : public Panel
{
public:
	PanelResources();
	virtual ~PanelResources();

	void Draw() override;


	UID DrawResourceType(Resource::Type type, bool opened = false);

private:
	UID  DrawResourceType(Resource::Type type, void (PanelResources::*popup)(void), bool opened);

    void DrawMeshPopup();
    void DrawPlaneProperties();
    void DrawCylinderProperties();

    void DrawTextureProperties();

private:
    struct TextureParams
    {
        std::string file;
        std::string extension;
        bool        compressed = true;
        bool        mipmaps    = true;
        bool        srgb       = true;
    };

    TextureParams texture_params;

};

#endif  // __PANELRESOURCES_H__
