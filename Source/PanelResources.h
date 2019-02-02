#ifndef __PANELRESOURCES_H__
#define __PANELRESOURCES_H__

#include "Panel.h"
#include "Resource.h"

#include <vector>

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
    void DrawAnimationProperties();

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

};

#endif  // __PANELRESOURCES_H__
