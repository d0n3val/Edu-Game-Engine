#ifndef _GEOMETRY_BATCH_H_
#define _GEOMETRY_BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "OGL.h"
#include "HashString.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <set>

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

    enum { NUM_BUFFERS = 2 };

    typedef std::vector<DrawCommand>                                CommandList;
    typedef std::unordered_map<UID, MeshData>                       MeshList;
    typedef std::unordered_map<const ComponentMeshRenderer*, uint>  ObjectList; // second is the instance index
    typedef std::set<const ComponentMeshRenderer*>                  UpdateList;
    typedef std::vector<PerInstance>                                InstanceList;

    uint                            attrib_flags = 0;

    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo[ATTRIB_COUNT];
    std::unique_ptr<Buffer>         tpose_positions;
    std::unique_ptr<Buffer>         tpose_normals;
    std::unique_ptr<Buffer>         tpose_tangents;
    std::unique_ptr<Buffer>         transformSSBO[NUM_BUFFERS];
    std::unique_ptr<Buffer>         materialSSBO;
    std::unique_ptr<Buffer>         instanceSSBO;
    std::unique_ptr<Buffer>         skinning[NUM_BUFFERS];
    std::unique_ptr<Buffer>         morphBuffer;
    std::unique_ptr<TextureBuffer>  morphTexture;
    std::unique_ptr<Buffer>         morphWeights[NUM_BUFFERS];
    std::unique_ptr<Buffer>         commandBuffer;

    float4x4*                       transformsData[NUM_BUFFERS];
    float4x4*                       skinningData[NUM_BUFFERS];
    float*                          morphWeightsData[NUM_BUFFERS];
    void*                           sync[NUM_BUFFERS] = { nullptr, nullptr };

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
    Program*                        skinningProgram = nullptr;

public:

    explicit GeometryBatch(const HashString& tag, Program* program);
    ~GeometryBatch() = default;
   
    bool               CanAdd            (const ComponentMeshRenderer* object) const;
    void               Add               (const ComponentMeshRenderer* object);
    void               Remove            (const ComponentMeshRenderer* object);

    void               MarkForUpdate     (const ComponentMeshRenderer* object);
    void               Render            (const ComponentMeshRenderer* object);
    void               DoRender          (uint flags);
    void               DoUpdate          ();

    bool               HasCommands       () const { return !commands.empty();  }
    bool               IsEmpty           () const { return objects.empty(); }
    const HashString&  GetTagName        () const { return tagName; }

    void               CreateRenderData  ();
    void               OnMaterialModified(UID materialID);
    
private:

    void ClearRenderData      ();

    void CreateVertexBuffers  ();
    void CreateMaterialBuffer ();
    void CreateInstanceBuffer ();
    void CreateMorphBuffer    ();
    void CreateTransformBuffer();
    void CreateCommandBuffer();
    void UpdateModels();
    void UpdateSkinning();
};

#endif /* _GEOMETRY_BATCH_H_ */
