#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Module.h"
#include "Math.h"

#include<vector>

class GameObject;
class ComponentCamera;
class ComponentMesh;
class ComponentMaterial;
class ComponentParticleSystem;
class ComponentTrail;
class ResourceMaterial;
class ResourceMesh;

class ModuleRenderer : public Module
{
    struct TRenderInfo
    {
        const char*              name         = nullptr;
        GameObject*              go           = nullptr;
        ComponentMesh*           mesh         = nullptr;
        ComponentParticleSystem* particles    = nullptr;
        ComponentTrail*          trail        = nullptr;
        float                    distance     = 0.0f;
        float                    layer        = 0.0f;
    };

    struct TNearestMesh
    {
        bool operator()(const TRenderInfo& info, const TRenderInfo& new_info)
        {
            return info.distance < new_info.distance;
        }
    };

    struct TFarthestMesh
    {
        bool operator()(const TRenderInfo& info, const TRenderInfo& new_info)
        {
            return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
        }
    };

	typedef std::vector<TRenderInfo> NodeList;
	typedef std::pair<uint, uint> Size;

    NodeList opaque_nodes;
    NodeList transparent_nodes;

    uint post_vbo       = 0;
    uint post_vao       = 0;
    uint sky_vbo        = 0;
    uint sky_vao        = 0;
    uint sky_cubemap    = 0;
    uint sky_irradiance = 0;
    uint sky_prefilter  = 0;
    uint sky_brdf       = 0;
    uint camera_buffer  = 0;

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
        float       near_distance   = 0.0f;
        float       far_distance    = 0.0f;
        AABB        aabb;
        OBB         world_bb;
        Frustum     frustum;
        NodeList    casters;
        uint        tick            = 0;
        uint        period          = 1;
    };

    uint bloom_blur_fbo = 0;
    uint bloom_blur_tex = 0;
    uint bloom_width    = 0;
    uint bloom_height   = 0;

    enum EShadows { CASCADE_COUNT = 3 };

    ShadowMap cascades[CASCADE_COUNT];

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                Init                        (Config* config = nullptr) override;
    void                Draw                        (ComponentCamera* camera, unsigned fbo, unsigned width, unsigned height);
    void                Postprocess                 (unsigned screen_texture, unsigned bloom_texture, unsigned fbo, unsigned width, unsigned height);

	void                DrawDebug                   () override;

    unsigned            GetShadowMap                (uint index) const { return cascades[index].tex; }
    unsigned            GetShadowMapWidth           (uint index) const { return cascades[index].width; }
    unsigned            GetShadowMapHeight          (uint index) const { return cascades[index].height; }

private:

    void                ShadowPass                  (ComponentCamera* camera, unsigned width, unsigned height);
    void                ColorPass                   (const float4x4& proj, const float4x4& view, const float3& view_pos, 
                                                     unsigned fbo, unsigned width, unsigned height);

    void                LoadDefaultShaders          ();
    void                CreatePostprocessData       ();
    void                CreateSkybox                ();

    void                DrawSkybox                  (const float4x4& proj, const float4x4& view);
    void                DrawNodes                   (const NodeList& nodes, void (ModuleRenderer::*drawer)(const TRenderInfo& ));

    void                DrawColor                   (const TRenderInfo& render_info);
    void                DrawShadow                  (const TRenderInfo& render_info);
    void                DrawMeshColor               (const ComponentMesh* mesh);
    void                DrawParticles               (ComponentParticleSystem* particles);
    void                DrawTrails                  (ComponentTrail* trail);

    void                CollectObjects              (const float3& camera_pos, GameObject* go);
    void                UpdateLightUniform          () const;

    void                BlurShadow                  (uint index);
    void                ComputeDirLightShadowVolume (ComponentCamera* camera, uint index);
    void                CalcLightCameraBBox         (const Quat& light_rotation, const ComponentCamera* camera, float near_distance, float far_distance, AABB& aabb);
    void                CalcLightObjectsBBox        (const Quat& light_rotation, AABB& aabb, NodeList& casters);
    void                DrawClippingSpace           (const math::float4x4& proj, const math::float4x4& view) const;
    void                GetClippingPoints           (const math::float4x4& proj, const math::float4x4& view, math::float3 points[8]) const;

    void                DebugDrawParticles          ();
    void                DebugDrawTangentSpace       ();
    void                DebugDrawTangentSpace       (const ResourceMesh* mesh, const float4x4& transform);
    void                DebugDrawAnimation          ();
    void                DebugDrawAnimation          (const GameObject* go);
    void                DebugDrawHierarchy          (const GameObject* go);

    void                GenerateShadowFBO           (ShadowMap& map, unsigned width, unsigned height);
    void                GenerateBloomFBO            (unsigned width, unsigned height);
    float4x4            SetOrtho                    (float left, float right, float bottom, float top, float _near, float _far);
};


#endif /* _RENDERER_H_ */
