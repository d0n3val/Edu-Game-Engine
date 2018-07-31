#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Module.h"
#include "Math.h"

#include<vector>

class GameObject;
class ComponentMesh;

class ModuleRenderer : public Module
{
	typedef std::vector<GameObject*> NodeList;
    typedef std::vector<ComponentMesh*> MeshList;

    enum ShaderVariation 
    {
        SKINNING          = 1 << 0,
        PIXEL_LIGHTING    = 1 << 1,
        NORMAL_MAP        = 1 << 2,
        SPECULAR_MAP      = 1 << 3,
        LIGHT_DIRECTIONAL = 1 << 4,
        RECEIVE_SHADOWS   = 1 << 5
    };

    enum UniformBlock
    {
        UNIFORM_CAMERA = 0,
        UNIFORM_LIGHT,
        UNIFORM_COUNT
    };

    struct R2TInfo
    {
        unsigned  fbo   = 0;
        unsigned  depth = 0;
        unsigned  tex   = 0;
        uint2     size = uint2(0, 0);
    };

    NodeList        draw_nodes;
    MeshList        tmp_meshes;

    static unsigned uniforms[UNIFORM_COUNT];
    static unsigned renderer_count;

    R2TInfo color;
    R2TInfo shadow;

public:

    Renderer();
    ~Renderer();

    void                Draw                    (unsigned width, unsigned height);
    void                DebugDrawTangentSpace   (float size);
    
    unsigned            GetNumDrawNodes         () const;
    const GameObject*   GetDrawNode             (unsigned index) const;

    unsigned            GetColorFB              () const;
    unsigned            GetColorFBTexture       () const;
    const uint2&        GetColorFBSize          () const;
    unsigned            GetShadowFBTexture      () const;
    const uint2&        GetShadowFBSize         () const;

private:

    void                LoadDefaultShaders      ();
    void                LoadShadowShaders       ();
    void                CollectNodes            ();
    void                CollectNodesRec         (GameObject* node);
    //\todo: void                DrawSkybox              ();
    void                DrawNodes               (void (Renderer::*drawer)(const float4x4& transform, ComponentMesh* mesh));
    void                DrawMeshColor           (const float4x4& transform, ComponentMesh* mesh);
    void                DrawMeshShadow          (const float4x4& transform, ComponentMesh* mesh);
    void                ShadowPass              (unsigned width, unsigned height);
    void                ColorPass               (unsigned width, unsigned height);
    void                GenerateFBOTexture      (R2TInfo& info, unsigned width, unsigned height, bool aColor); 
    void                DrawClippingSpace       (const float4x4& proj, const float4x4& view) const;
    void                GetClippingPoints       (const float4x4& proj, const float4x4& view, float3 points[8]) const;
    void                UpdateLightUniform      () const;
    void                UpdateCameraUniform     () const;
    void                CalcLightSpaceBBox      (const float4& light_rotation, Scene::AABB& aabb) const;
};


inline unsigned Renderer::GetNumDrawNodes() const
{
    return draw_nodes.size();
}

inline const GameObject* Renderer::GetDrawNode(unsigned index) const
{
    return draw_nodes[index];
}

inline unsigned Renderer::GetColorFBTexture() const
{
    return color.tex;
}

inline const uint2& Renderer::GetColorFBSize() const
{
    return color.size;
}

inline unsigned Renderer::GetShadowFBTexture() const
{
    return shadow.tex;
}

inline const uint2& Renderer::GetShadowFBSize() const
{
    return shadow.size;
}

inline unsigned Renderer::GetColorFB() const
{
    return color.fbo;
}

#endif /* _RENDERER_H_ */
