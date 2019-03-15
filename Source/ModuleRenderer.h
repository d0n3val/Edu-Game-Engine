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
class ResourceMaterial;
class ResourceMesh;

class ModuleRenderer : public Module
{
    struct TRenderInfo
    {
        const char*              name         = nullptr;
        GameObject*              go           = nullptr;
        ComponentMesh*           mesh         = nullptr;
        ComponentMaterial*       material     = nullptr;
        ComponentParticleSystem* particles    = nullptr;
        float                    distance     = 0.0f;
        float4x4                 transform    = float4x4::identity;
        const float4x4*          skin_palette = nullptr;
    };

    struct TNearestMesh
    {
        bool operator()(const TRenderInfo& info, float distance)
        {
            return info.distance < distance;
        }
    };

    struct TFarthestMesh
    {
        bool operator()(const TRenderInfo& info, float distance)
        {
            return info.distance > distance;
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

    void                DrawMeshColor           (const TRenderInfo& render_info);
    void                DrawMeshColor           (const ResourceMesh* mesh, const ResourceMaterial* material, const float4x4& transform, const float4x4* skin_palette);
    void                DrawParticles           (const ComponentParticleSystem* particles, const float4x4& transform);
    void                UpdateLightUniform      () const;
    void                CollectObjects          (const float3& camera_pos, GameObject* go);

    void                CreateCameraBuffer      ();

    void                DebugDrawTangentSpace   ();
    void                DebugDrawTangentSpace   (const ResourceMesh* mesh, const float4x4& transform);
    void                DebugDrawAnimation      ();
    void                DebugDrawAnimation      (const GameObject* go);
    void                DebugDrawHierarchy      (const GameObject* go);
};


#endif /* _RENDERER_H_ */
