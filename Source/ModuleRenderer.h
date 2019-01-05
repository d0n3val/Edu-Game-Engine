#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Module.h"
#include "Math.h"

#include<vector>

class GameObject;
class ComponentCamera;
class ComponentMesh;
class ComponentMaterial;
class ResourceMaterial;

class ModuleRenderer : public Module
{
    struct TRenderInfo
    {
        const char*         name = nullptr;
        ComponentMesh*      mesh = nullptr;
        ComponentMaterial*  material = nullptr;
        float               distance = 0.0f;
        float4x4            transform = float4x4::identity;
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

    unsigned post_vbo  = 0;
    unsigned post_vao  = 0;

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                Init                    (Config* config = nullptr) override;
    void                Draw                    (ComponentCamera* camera, unsigned fbo, unsigned width, unsigned height);
    void                Postprocess             (unsigned screen_texture, unsigned fbo, unsigned width, unsigned height);

	void                DrawDebug               () override;
    
private:

    void                LoadDefaultShaders      ();

    void                DrawNodes               (void (ModuleRenderer::*drawer)(const TRenderInfo& , const float4x4&, const float4x4&, const float3&), 
                                                 const float4x4& projection, const float4x4& view, const float3& view_pos);

    void                DrawMeshColor           (const TRenderInfo& render_info, const float4x4& projection, const float4x4& view, const float3& view_pos);
    void                UpdateMaterialUniform   (const ResourceMaterial* material) const;
    void                UpdateLightUniform      () const;
    void                CollectObjects          (const float3& camera_pos, GameObject* go);
};


#endif /* _RENDERER_H_ */
