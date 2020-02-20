#ifndef __COMPONENT_MESHRENDERER_H__
#define __COMPONENT_MESHRENDERER_H__

#include "Component.h"

class ResourceMesh;
class ResourceMaterial;

class ComponentMeshRenderer : public Component
{
public:

    enum ERenderMode
    {
        RENDER_OPAQUE = 0,
        RENDER_TRANSPARENT,
        RENDER_COUNT
    };
    
public:
    explicit ComponentMeshRenderer(GameObject* go);
    ~ComponentMeshRenderer();

	bool                    SetMeshRes          (UID uid);
    const ResourceMesh*     GetMeshRes          () const;
    ResourceMesh*           GetMeshRes          ();

    bool                    SetMaterialRes      (UID uid);
    const ResourceMaterial* GetMaterialRes      () const;
    ResourceMaterial*       GetMaterialRes      ();

    virtual void            OnSave              (Config& config) const override;
    virtual void            OnLoad              (Config* config) override;

    // mesh

    void                    Draw                () const;
    void                    DrawShadowPass      () const;

	void                    GetBoundingBox      (AABB& box) const override;

    static Types            GetClassType        () { return MeshRenderer; }

    bool                    GetVisible          () const { return visible; }
    void                    SetVisible          (bool v) { visible = v; }

    const float4x4*         UpdateSkinPalette   () const;
    void                    SetRootUID          (UID r) { root_uid = r; }

    // material

    bool                    GetDDTangent        () const { return debug_draw_tangent; }
    void                    SetDDTangent        (bool enable) { debug_draw_tangent = enable; }

    bool                    CastShadows         () const { return cast_shadows; }
    void                    SetCastShadows      (bool casts) { cast_shadows = casts; }
    bool                    RecvShadows         () const { return recv_shadows; }

    ERenderMode             RenderMode          () const { return render_mode; }
    void                    SetRenderMode       (ERenderMode mode) { render_mode = mode; }

    void                    SetMorphTarget      (uint index) const;

private:

    // mesh 
	UID mesh_resource              = 0;
    mutable float4x4* skin_palette = nullptr;
    const GameObject** node_cache  = nullptr;
    bool visible                   = true;
    UID root_uid                   = 0;

    // material
    UID  material_resource         = 0;
    bool cast_shadows              = true;
    bool recv_shadows              = true;
    bool debug_draw_tangent        = false;
    ERenderMode render_mode        = RENDER_OPAQUE;
};

#endif /* __COMPONENT_MESHRENDERER_H__ */
