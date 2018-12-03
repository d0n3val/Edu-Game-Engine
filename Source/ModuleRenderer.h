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
	typedef std::vector<GameObject*> NodeList;
	typedef std::pair<uint, uint> Size;

    NodeList draw_nodes;

public:

    explicit ModuleRenderer();
    ~ModuleRenderer();

	bool                Init                    (Config* config = nullptr);
    void                Draw                    (ComponentCamera* camera, unsigned width, unsigned height);
    
    unsigned            GetNumDrawNodes         () const;
    const GameObject*   GetDrawNode             (unsigned index) const;

private:

    void                LoadDefaultShaders      ();

    void                DrawNodes               (void (ModuleRenderer::*drawer)(const float4x4&, const ComponentMesh*, const ComponentMaterial*,
                                                 const float4x4&, const float4x4&), const float4x4& projection, const float4x4& view);


    void                DrawMeshColor           (const float4x4& transform, const ComponentMesh* mesh, const ComponentMaterial* material, 
												 const float4x4& projection, const float4x4& view);
    void                UpdateMaterialUniform   (const ResourceMaterial* material) const;
};


inline unsigned ModuleRenderer::GetNumDrawNodes() const
{
    return draw_nodes.size();
}

inline const GameObject* ModuleRenderer::GetDrawNode(unsigned index) const
{
    return draw_nodes[index];
}

#endif /* _RENDERER_H_ */
