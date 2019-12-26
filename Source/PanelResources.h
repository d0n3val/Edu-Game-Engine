#ifndef __PANELRESOURCES_H__
#define __PANELRESOURCES_H__

#include "Panel.h"
#include "Resource.h"

#include <vector>
#include <set>

class PanelResources : public Panel
{
public:
	PanelResources();
	virtual ~PanelResources();

	void Draw() override;


private:
	void  DrawResourceType(Resource::Type type, void (PanelResources::*popup)(Resource::Type));

    void DrawMeshPopup(Resource::Type type);
    void DrawResourcePopup(Resource::Type type);
    void DrawPlaneProperties();
    void DrawCylinderProperties();
    void DrawSphereProperties();

    void DrawTextureProperties();
    void DrawAnimationProperties();
    void ManageSelection(const std::vector<const Resource*>& resources, uint current, bool append, bool multiple, uint pivot, bool popup);

private:

    struct TextureParams
    {
        std::string file;
        bool        compressed = true;
        bool        mipmaps    = true;
        bool        srgb       = true;

        void Reset() 
        {
            file.clear();
            compressed = true;
            mipmaps    = true;
            srgb       = true;
        }
    };
    
    struct AnimationClip
    {
        char        name[128];
        uint        first = 0;
        uint        last  = uint(INT_MAX);

        AnimationClip()
        {
            name[0] = 0;
        }

        void Reset() 
        {
            name[0] = 0;
            first   = 0;
            last    = uint(INT_MAX);
        }
    };

    struct AnimationParams
    {
        std::string file;
        char        clip_names[128];
        std::vector<AnimationClip> clips;

        AnimationParams()
        {
            clips.resize(1);
        }

        void Reset() 
        {
            file.clear();
            clips.clear();
            clips.resize(1);
        }
    };

    TextureParams texture_params;
    AnimationParams animation_params;

	bool waiting_to_load_file      = false;
    Resource::Type waiting_to_load = Resource::unknown;
    std::set<UID> selection;
    Resource::Type multiple_select_type = Resource::unknown;
    uint multiple_select_pivot          = 0;

};

#endif  // __PANELRESOURCES_H__
