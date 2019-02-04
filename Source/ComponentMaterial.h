#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

struct TextureInfo;
class ResourceMaterial;

class ComponentMaterial : public Component
{
public:
    enum ERenderMode
    {
        RENDER_OPAQUE = 0,
        RENDER_TRANSPARENT,
        RENDER_COUNT
    };
public:

	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	bool                    SetResource (UID uid);
    const ResourceMaterial* GetResource () const;
    ResourceMaterial*       GetResource ();

    bool                    GetDDTangent    () const { return debug_draw_tangent; }
    void                    SetDDTangent    (bool enable) { debug_draw_tangent = enable; }

    bool                    CastShadows     () const { return cast_shadows; }
    bool                    RecvShadows     () const { return recv_shadows; }

    ERenderMode             RenderMode      () const { return render_mode; }
    void                    SetRenderMode   (ERenderMode mode) { render_mode = mode; }

	void                    OnSave      (Config& config) const override;
	void                    OnLoad      (Config* config) override;

    static Types            GetClassType() { return Material; }

private:

    UID  resource          = 0;
    bool cast_shadows       = true;
    bool recv_shadows       = true;
    bool debug_draw_tangent = false;
    ERenderMode render_mode = RENDER_OPAQUE;
    
};

#endif // __COMPONENT_MESH_H__
