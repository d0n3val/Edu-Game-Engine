#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Module.h"
#include "Math.h"
#include "RenderList.h"
#include "OGL.h"

#include<vector>

class GameObject;
class ComponentCamera;
class ComponentMeshRenderer;
class ComponentParticleSystem;
class ComponentTrail;
class ResourceMaterial;
class ResourceMesh;
class BatchManager;
class QuadtreeNode;
class Postprocess;
class Texture2D;
class Framebuffer;

class ModuleRenderer : public Module
{
    RenderList render_list;

    struct ShadowMap
    {
        float4x4    proj            = float4x4::identity;
        float4x4    view            = float4x4::identity;
        uint        fbo             = 0;
        uint        sq_fbo          = 0;
        uint        blur_fbo_0      = 0;
        uint        blur_fbo_1      = 0;
        uint        width           = 0; 
        uint        height          = 0; 
        uint        tex             = 0;
        uint        sq_tex          = 0;
        uint        blur_tex_0      = 0;
        uint        blur_tex_1      = 0;
        AABB        aabb;
        OBB         world_bb;
        Frustum     frustum;
        NodeList    casters;
        uint        tick            = 0;
    };

    enum EShadows { CASCADE_COUNT = 3 };

    ShadowMap cascades[CASCADE_COUNT];

    struct CameraData
    {
        float4x4 proj = float4x4::identity;
        float4x4 view = float4x4::identity;
    };

    std::unique_ptr<BatchManager> batch_manager;
    std::unique_ptr<Postprocess>  postProcess;
    std::unique_ptr<Buffer>       cameraUBO;
    std::unique_ptr<Buffer>       lightsUBO;
    CameraData                    cameraData;

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                Init                        (Config* config = nullptr) override;
    void                Draw                        (ComponentCamera* camera, ComponentCamera* culling, unsigned fbo, unsigned width, unsigned height);
    void                DrawForSelection            (ComponentCamera* camera);

	void                DrawDebug                   () override;

    unsigned            GetShadowMap                (uint index) const { return cascades[index].tex; }
    unsigned            GetShadowMapWidth           (uint index) const { return cascades[index].width; }

    BatchManager*       GetBatchMananger            () const { return batch_manager.get(); }
    Postprocess*        GetPostprocess              () const { return postProcess.get(); }

private:

    void                ShadowPass                  (ComponentCamera* camera, unsigned width, unsigned height);
    void                ColorPass                   (const float4x4& proj, const float4x4& view, const float3& view_pos, 
                                                     unsigned fbo, unsigned width, unsigned height);
    void                SelectionPass               (const float4x4& proj, const float4x4& view);

    void                LoadDefaultShaders          ();

    void                DrawBatches                 (NodeList& nodes, uint render_flags);
    void                DrawNodes                   (const NodeList& nodes, void (ModuleRenderer::*drawer)(const TRenderInfo& ));

    void                DrawColor                   (const TRenderInfo& render_info);
    void                DrawShadow                  (const TRenderInfo& render_info);
    void                DrawMeshColor               (ComponentMeshRenderer* mesh);
    void                DrawParticles               (ComponentParticleSystem* particles);
    void                DrawTrails                  (ComponentTrail* trail);
    void                DrawSelection               (const TRenderInfo& render_info);

    void                UpdateLightUniform          () ;

    void                BlurShadow                  (uint index);
    void                ComputeDirLightShadowVolume (ComponentCamera* camera, uint index);
    void                CalcLightCameraBBox         (const Quat& light_rotation, const ComponentCamera* camera, float near_distance, float far_distance, AABB& aabb);
    void                CalcLightObjectsBBox        (const Quat& light_rotation, const float3& light_dir, AABB& aabb, NodeList& casters);
    void                CalcLightObjectsBBox        (const float4x4& light_mat, const AABB& camera_aabb, AABB& aabb, NodeList& casters, const NodeList& objects);
    void                DrawClippingSpace           (const math::float4x4& proj, const math::float4x4& view) const;
    void                GetClippingPoints           (const math::float4x4& proj, const math::float4x4& view, math::float3 points[8]) const;

    void                DebugDrawOBB                ();
    void                DebugDrawOBB                (const NodeList& objects);
    void                DebugDrawTangentSpace       ();
    void                DebugDrawTangentSpace       (const ResourceMesh* mesh, const float4x4& transform);
    void                DebugDrawAnimation          ();
    void                DebugDrawAnimation          (const GameObject* go);
    void                DebugDrawHierarchy          (const GameObject* go);

    void                GenerateShadowFBO           (ShadowMap& map, unsigned width, unsigned height);
    float4x4            SetOrtho                    (float left, float right, float bottom, float top, float _near, float _far);
    float4x4            SetFrustum                  (float left, float right, float bottom, float top, float _near, float _far);
    float4x4            ComputePerspShadowMtx       (const float3& view_pos, const float3& view_dir, const float3& light_dir, const float3x3& light_view);
};


#endif /* _RENDERER_H_ */
