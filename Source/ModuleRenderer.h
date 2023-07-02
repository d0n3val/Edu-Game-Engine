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
class ForwardPass;
class ScreenSpaceAO;
class GBufferExportPass;
class DeferredResolvePass;
class DeferredResolveProxy;
class DeferredDecalPass;
class ShadowmapPass;
class CascadeShadowPass;
class ParticlePass;
class FxaaPass;
class FogPass;
class LinePass;
class DepthRangePass;
class PlanarReflectionPass;
class SpotConePass;
class CameraUBO;


class ModuleRenderer : public Module
{
    RenderList render_list;

    std::unique_ptr<BatchManager>         batch_manager;
    std::unique_ptr<Postprocess>          postProcess;
    std::unique_ptr<ForwardPass>          forward;
    std::unique_ptr<GBufferExportPass>    exportGBuffer;
    std::unique_ptr<DeferredResolvePass>  deferredResolve;
    std::unique_ptr<DeferredResolveProxy> deferredProxy;
    std::unique_ptr<DeferredDecalPass>    decalPass;
    std::unique_ptr<ScreenSpaceAO>        ssao;
    std::unique_ptr<FxaaPass>             fxaa;
    std::unique_ptr<FogPass>              fogPass;
    std::unique_ptr<ShadowmapPass>        shadowmapPass;
    std::unique_ptr<CascadeShadowPass>    cascadeShadowPass;
    std::unique_ptr<LinePass>             linePass;
    std::unique_ptr<ParticlePass>         particlePass;
    std::unique_ptr<DepthRangePass>       depthRangePass;
    std::unique_ptr<PlanarReflectionPass> planarPass;
    std::unique_ptr<SpotConePass>         spotConePass;
    std::unique_ptr<CameraUBO>            cameraUBO;
    std::unique_ptr<Program>              primitiveProgram;
    std::unique_ptr<Program>              probeProgram;
    std::unique_ptr<Program>              probeLodProgram;

public:

    enum DrawFlags
    {
        DRAW_IBL = 1 << 0,
        DRAW_PLANAR = 1 << 1
    };

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                    Init                        (Config* config = nullptr) override;
    void                    Draw                        (ComponentCamera* camera, ComponentCamera* culling,  Framebuffer* frameBuffer, unsigned width, unsigned height, uint flags = 0);
    void                    DrawForSelection            (ComponentCamera* camera);

	void                    DrawDebug                   () override;

    BatchManager*           GetBatchManager             () const { return batch_manager.get(); }
    Postprocess*            GetPostprocess              () const { return postProcess.get(); }
    ScreenSpaceAO*          GetScreenSpaceAO            () const { return ssao.get(); }
    GBufferExportPass*      GetGBufferExportPass        () const { return exportGBuffer.get(); }
    FxaaPass*               GetFxaaPass                 () const { return fxaa.get(); }
    ShadowmapPass*          GetShadowmapPass            () const { return shadowmapPass.get(); }
    CascadeShadowPass*      GetCascadeShadowPass        () const { return cascadeShadowPass.get(); }
    PlanarReflectionPass*   GetPlanarPass               () const { return planarPass.get(); }
    CameraUBO*              GetCameraUBO                () const { return cameraUBO.get(); }

    Program*                GetPrimitivesProgram        () const { return primitiveProgram.get(); }

private:

    void                RenderForward               (ComponentCamera* camera, Framebuffer* frameBuffer, unsigned width, unsigned height);
    void                RenderDeferred              (ComponentCamera* camera, ComponentCamera* culling, Framebuffer* frameBuffer, unsigned width, unsigned height, uint flags);
    void                RenderVFX                   (ComponentCamera* camera, ComponentCamera* culling, Framebuffer* frameBuffer, unsigned width, unsigned height);


    void                SelectionPass               (const float4x4& proj, const float4x4& view);

    void                LoadDefaultShaders          ();

    void                DrawSelection               (const TRenderInfo& render_info);

    void                DebugDrawOBB                ();
    void                DebugDrawOBB                (const NodeList& objects);
    void                DebugDrawTangentSpace       ();
    void                DebugDrawTangentSpace       (const ResourceMesh* mesh, const float4x4& transform);
    void                DebugDrawAnimation          ();
    void                DebugDrawAnimation          (const GameObject* go);
    void                DebugDrawHierarchy          (const GameObject* go);

    float4x4            SetOrtho                    (float left, float right, float bottom, float top, float _near, float _far);
    float4x4            SetFrustum                  (float left, float right, float bottom, float top, float _near, float _far);
    void                UpdateCameraUBO             (ComponentCamera *camera);

    void                DrawAreaLights              (ComponentCamera* camera, Framebuffer* frameBuffer);
    bool                CreateAreaLightProgram      ();
    bool                CreateProbeProgram          ();
};

#endif /* _RENDERER_H_ */
