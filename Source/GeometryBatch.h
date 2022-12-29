#ifndef _GEOMETRY_BATCH_H_
#define _GEOMETRY_BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
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
        uint numTargets     = 0;
        uint baseTarget     = 0;
	};

    struct PerInstance
    {
        uint numBones         = 0;
        uint baseBone         = 0;
        uint numTargets       = 0;
        uint baseTarget       = 0;
        uint baseTargetWeight = 0;
        uint targetStride     = 0;
        uint normalsStride    = 0;
        uint tangentsStride   = 0;
    };

    struct DrawCommand
    {
        uint count         = 0;
        uint instanceCount = 0;
        uint firstIndex    = 0;
        uint baseVertex    = 0;
        uint baseInstance  = 0;
    }; 

    typedef std::vector<DrawCommand>                                CommandList;
    typedef std::unordered_map<UID, MeshData>                       MeshList;
    typedef std::unordered_map<const ComponentMeshRenderer*, uint>  ObjectList; // second is the instance index
    typedef std::vector<const ComponentMeshRenderer*>               UpdateList;
    typedef std::vector<PerInstance>                                InstanceList;

    uint                            attrib_flags = 0;

    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo;
    std::unique_ptr<Buffer>         drawIdVBO;
    std::unique_ptr<Buffer>         transformSSBO[2];
    std::unique_ptr<Buffer>         materialSSBO;
    std::unique_ptr<Buffer>         instanceSSBO;
    std::unique_ptr<Buffer>         skinning[2];
    std::unique_ptr<Buffer>         morphBuffer;
    std::unique_ptr<TextureBuffer>  morphTexture;
    std::unique_ptr<Buffer>         morphWeights[2];
    std::unique_ptr<Buffer>         commandBuffer;

    float4x4*                       transformsData[2];
    float4x4*                       skinningData[2];
    float*                          morphWeightsData[2];
    void*                           sync[2] = { nullptr, nullptr };

    uint                            commandBufferSize = 0;
    uint                            totalBones = 0;
    uint                            totalTargets = 0;
    uint                            frameCount = 0;

    MeshList                        meshes;
    ObjectList                      objects;
    InstanceList                    instances;

    CommandList                     commands;
    UpdateList                      modelUpdates;

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
    void               DoRender          (uint flags);

    bool               HasCommands       () const { return !commands.empty();  }
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
    void CreateMorphBuffer    ();
    void CreateTransformBuffer();
    void CreateCommandBuffer();
    void UpdateModels();
};

#endif /* _GEOMETRY_BATCH_H_ */
