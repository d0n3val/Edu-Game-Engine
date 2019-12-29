#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

class ResourceMesh;

class ComponentMesh : public Component
{
public:
    explicit ComponentMesh(GameObject* go);
    ~ComponentMesh();

	bool                SetResource         (UID uid);
    const ResourceMesh* GetResource         () const;
    ResourceMesh*       GetResource         ();
    virtual void        OnSave              (Config& config) const override;
    virtual void        OnLoad              (Config* config) override;

    void                Draw                () const;
    void                DrawShadowPass      () const;

	void                GetBoundingBox      (AABB& box) const override;

    static Types        GetClassType        () { return Mesh; }

    bool                GetVisible          () const { return visible; }
    void                SetVisible          (bool v) { visible = v; }

    const float4x4*     UpdateSkinPalette   () const;
    void                SetRootUID          (UID r) { root_uid = r; }

private:

	UID resource                   = 0;
    mutable float4x4* skin_palette = nullptr;
    const GameObject** node_cache  = nullptr;
    bool visible                   = true;
    UID root_uid                   = 0;
};

#endif /* __COMPONENT_MESH_H__ */
