#ifndef __COMPONENT_MESHRENDERER_H__
#define __COMPONENT_MESHRENDERER_H__

#include "Component.h"
#include "ResourceModel.h"
#include "HashString.h"

class ResourceMesh;
class ResourceMaterial;
class BatchManager;
class DefaultShader;

enum ERenderMode
{
    RENDER_OPAQUE = 0,
    RENDER_TRANSPARENT,
    RENDER_COUNT
};

class ComponentMeshRenderer : public Component
{
public:
    explicit ComponentMeshRenderer(GameObject* go);
    ~ComponentMeshRenderer();

	bool                    SetMeshRes              (UID uid);
    const ResourceMesh*     GetMeshRes              () const;
    ResourceMesh*           GetMeshRes              ();
    UID                     GetMeshUID              () const { return mesh_resource; }

    bool                    SetMaterialRes          (UID uid);
    const ResourceMaterial* GetMaterialRes          () const;
    ResourceMaterial*       GetMaterialRes          ();
    UID                     GetMaterialUID          () const {return material_resource; }

    bool                    SetSkinInfo             (const ResourceModel::Skin& skin, GameObject** gos);

    virtual void            OnSave                  (Config& config) const override;
    virtual void            OnLoad                  (Config* config) override;

    // mesh

	void                    GetBoundingBox          (AABB& box) const override;

    static Types            GetClassType            () { return MeshRenderer; }

    bool                    GetVisible              () const { return visible; }
    void                    SetVisible              (bool v) { visible = v; }

    uint32_t                GetNumBones() const { return numBones; }
    void                    UpdateSkinPalette       (float4x4* palette) const;
    void                    UpdateCPUMorphTargets   () const;

    // material

    bool                    GetDDTangent            () const { return debug_draw_tangent; }
    void                    SetDDTangent            (bool enable) { debug_draw_tangent = enable; }

    bool                    CastShadows             () const { return cast_shadows; }
    void                    SetCastShadows          (bool casts) { cast_shadows = casts; }
    bool                    RecvShadows             () const { return recv_shadows; }

    ERenderMode             RenderMode              () const { return render_mode; }
    void                    SetRenderMode           (ERenderMode mode) { render_mode = mode; }

    void                    SetMorphTargetWeight    (uint index, float weight) { dirty_morphs = dirty_morphs || morph_weights[index] != weight; morph_weights[index] = weight;  }
    float                   GetMorphTargetWeight    (uint index) const { return morph_weights[index]; }
    const float*            GetMorphTargetWeights   () const { return morph_weights.get(); }

    const HashString&       GetBatchName            () const {return batch_name; }
    void                    SetBatchName            (const HashString& name);

    uint                    GetBatchIndex           () const {return batch_index;}
    
private:

    // mesh 
	UID                      mesh_resource      = 0;
    const GameObject**       node_cache         = nullptr;
    bool                     visible            = true;

    // material
    UID                      material_resource  = 0;
    bool                     cast_shadows       = true;
    bool                     recv_shadows       = true;
    bool                     debug_draw_tangent = false;
    ERenderMode              render_mode        = RENDER_OPAQUE;
    std::unique_ptr<float[]> morph_weights;
    mutable bool             dirty_morphs       = false;

    struct Bone
    {
        GameObject* go;
        float4x4	bind = float4x4::identity;
    };

    std::unique_ptr<Bone[]> bones;
    uint32_t                numBones = 0;
    GameObject*             rootGO = nullptr;


    HashString               batch_name;
    uint                     batch_index        = UINT_MAX;
};

#endif /* __COMPONENT_MESHRENDERER_H__ */
