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

    unsigned post_vbo       = 0;
    unsigned post_vao       = 0;
    unsigned sky_vbo        = 0;
    unsigned sky_vao        = 0;
    unsigned sky_cubemap    = 0;
    unsigned sky_irradiance = 0;
    unsigned sky_prefilter  = 0;
    unsigned sky_brdf       = 0;
    unsigned camera_buffer  = 0;

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                Init                    (Config* config = nullptr) override;
    void                Draw                    (ComponentCamera* camera, unsigned fbo, unsigned width, unsigned height);
    void                Postprocess             (unsigned screen_texture, unsigned fbo, unsigned width, unsigned height);

	void                DrawDebug               () override;
    
private:

    void                LoadDefaultShaders      ();
    void                CreatePostprocessData   ();
    void                CreateSkybox            ();

    void                DrawSkybox              (const float4x4& proj, const float4x4& view);
    void                DrawNodes               (void (ModuleRenderer::*drawer)(const TRenderInfo& ));

    void                DrawColor               (const TRenderInfo& render_info);
    void                DrawMeshColor           (const ComponentMesh* mesh);
    void                DrawParticles           (ComponentParticleSystem* particles);
    void                DrawTrails              (ComponentTrail* trail);

    void                CollectObjects          (const float3& camera_pos, GameObject* go);
    void                UpdateLightUniform      () const;

    void                ComputeDirLightViewProj (float4x4& view, float4x4& proj);
    void                CalcLightSpaceBBox      (const Quat& light_rotation, AABB& aabb);
    void                DrawClippingSpace       (const math::float4x4& proj, const math::float4x4& view) const;
    void                GetClippingPoints       (const math::float4x4& proj, const math::float4x4& view, math::float3 points[8]) const;

    void                DebugDrawParticles      ();
    void                DebugDrawTangentSpace   ();
    void                DebugDrawTangentSpace   (const ResourceMesh* mesh, const float4x4& transform);
    void                DebugDrawAnimation      ();
    void                DebugDrawAnimation      (const GameObject* go);
    void                DebugDrawHierarchy      (const GameObject* go);
};


#endif /* _RENDERER_H_ */
