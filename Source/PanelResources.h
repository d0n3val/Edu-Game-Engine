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
	UID  DrawResourceType(Resource::Type type, void (PanelResources::*popup)(Resource::Type), bool opened);

    void DrawMeshPopup(Resource::Type type);
    void DrawResourcePopup(Resource::Type type);
    void DrawPlaneProperties();
    void DrawCylinderProperties();
    void DrawSphereProperties();

    void DrawTextureProperties();

private:

    struct TextureParams
    {
        std::string file;
        std::string extension;
        bool        compressed = true;
        bool        mipmaps    = true;
        bool        srgb       = true;

        void Reset() 
        {
            file.clear();
            extension.clear();
            compressed = true;
            mipmaps    = true;
            srgb       = true;
        }
    };

    TextureParams texture_params;
	bool waiting_to_load_file      = false;
    Resource::Type waiting_to_load = Resource::unknown;

};

#endif  // __PANELRESOURCES_H__
