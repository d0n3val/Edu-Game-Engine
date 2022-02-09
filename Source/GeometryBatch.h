#ifndef _GEOMETRY_BATCH_H_
#define _GEOMETRY_BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "TextureBatch.h"
#include "OGL.h"
#include "HashString.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class ComponentMeshRenderer;

class GeometryBatch
{
	struct MeshData
	{
        UID  uid            = 0;
		uint refCount       = 0;
        uint baseVertex     = 0;
        uint vertexCount    = 0;
        uint baseIndex      = 0;
        uint indexCount     = 0;
	};

    struct DrawCommand
    {
        uint count;
        uint instanceCount;
        uint firstIndex;
        uint baseVertex;
        uint baseInstance;
    }; 

    typedef std::vector<DrawCommand>                                CommandList;
    typedef std::unordered_map<UID, MeshData>                       MeshList;
    typedef std::unordered_map<const ComponentMeshRenderer*, uint>  ObjectList; // second is the instance index
    typedef std::vector<const ComponentMeshRenderer*>               UpdateList;

    uint                            attrib_flags = 0;

    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo;
    std::unique_ptr<Buffer>         drawIdVBO;
    std::unique_ptr<Buffer>         transformSSBO;
    std::unique_ptr<Buffer>         materialSSBO;
    std::unique_ptr<Buffer>         instanceSSBO;
    std::unique_ptr<Buffer>         commandBuffer;
    uint                            commandBufferSize = 0;

    MeshList                        meshes;
    ObjectList                      objects;
    TextureBatch                    textures;

    CommandList                     commands;
    UpdateList                      modelUpdates;
    float4x4*                       transforms = nullptr;

    HashString                      tagName;
    bool                            bufferDirty = false;

public:

    explicit GeometryBatch(const HashString& tag);
    ~GeometryBatch() = default;
   
    bool               CanAdd            (const ComponentMeshRenderer* object) const;
    void               Add               (const ComponentMeshRenderer* object);
    void               Remove            (const ComponentMeshRenderer* object);

    void               UpdateModel       (const ComponentMeshRenderer* object);
    void               Render            (const ComponentMeshRenderer* object);
    void               DoRender          (uint transformsIndex, uint materialsIndex, uint instancesIndex);

    bool               HasCommands       () const { return commands.empty();  }
    bool               IsEmpty           () const { return objects.empty(); }
    const HashString&  GetTagName        () const { return tagName; }

    void               CreateRenderData  ();
    void               OnMaterialModified(UID materialID);
private:

    void ClearRenderData      ();
    void GetVertexAttribs     (VertexAttrib* attribs, uint& count, uint& vertex_size) const;

    void CreateVertexBuffers  ();
    void CreateDrawIdBuffer   ();
    void CreateMaterialBuffer ();
    void CreateInstanceBuffer ();
    void CreateTransformBuffer();
    void CreateCommandBuffer();
    void UpdateModels();
};

#endif /* _GEOMETRY_BATCH_H_ */
