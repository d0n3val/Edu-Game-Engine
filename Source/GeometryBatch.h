#ifndef _GEOMETRY_BATCH_H_
#define _GEOMETRY_BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "OGL.h"
#include "HashString.h"
#include "ModuleRenderer.h"
#include "ComponentMeshRenderer.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <set>

class ComponentMeshRenderer;
class BatchDrawCommands;
struct BatchPrograms;

struct DrawCommand
{
    uint count = 0;
    uint instanceCount = 0;
    uint firstIndex = 0;
    uint baseVertex = 0;
    uint baseInstance = 0;
};

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

    struct PerInstanceGPU
    {
        uint indexCount = 0;
        uint baseIndex = 0;
        uint baseVertex = 0;
        uint numBones = 0;
        float4 obb[8];
    };

    typedef std::vector<DrawCommand>                                CommandList;
    typedef std::unordered_map<UID, MeshData>                       MeshList;
    typedef std::unordered_map<ComponentMeshRenderer*, uint>        ObjectList; // second is the instance index
    typedef std::vector<PerInstance>                                InstanceList;

    uint                            attrib_flags = 0;
    MaterialWorkFlow                materialWF = MetallicRoughness;

    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         instanceBuffer;
    std::unique_ptr<Buffer>         distanceBuffer;
    std::unique_ptr<Buffer>         indirectBuffer;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo[ATTRIB_COUNT];
    std::unique_ptr<Buffer>         tpose_positions;
    std::unique_ptr<Buffer>         tpose_normals;
    std::unique_ptr<Buffer>         tpose_tangents;
    std::unique_ptr<Buffer>         bone_indices;
    std::unique_ptr<Buffer>         bone_weights;
    std::unique_ptr<Buffer>         transformSSBO[ModuleRenderer::NUM_FLIGHT_FRAMES];
    std::unique_ptr<Buffer>         materialSSBO;
    std::unique_ptr<Buffer>         skinning[ModuleRenderer::NUM_FLIGHT_FRAMES];
    std::unique_ptr<Buffer>         morphBuffer;
    std::unique_ptr<TextureBuffer>  morphTexture;
    std::unique_ptr<Buffer>         morphWeights[ModuleRenderer::NUM_FLIGHT_FRAMES];
    std::unique_ptr<Buffer>         commandBuffer;

    float4x4*                       transformsData[ModuleRenderer::NUM_FLIGHT_FRAMES];
    float4x4*                       skinningData[ModuleRenderer::NUM_FLIGHT_FRAMES];
    float*                          morphWeightsData[ModuleRenderer::NUM_FLIGHT_FRAMES];

    uint                            commandBufferSize = 0;
    uint                            totalVertices = 0;
    uint                            totalIndices = 0;
    uint                            totalBones = 0;
    uint                            totalTargets = 0;

    MeshList                        meshes;
    ObjectList                      objects;
    InstanceList                    instances;

    CommandList                     commands;

    HashString                      tagName;
    bool                            bufferDirty = false;
    BatchPrograms*                  programs;
    uint32_t                        batchIndex = 0;
    ERenderMode                     renderMode = RENDER_OPAQUE;

public:

    explicit GeometryBatch(const HashString& tag, uint32_t index, BatchPrograms* programs);
    ~GeometryBatch();
   
    bool               CanAdd            (const ComponentMeshRenderer* object) const;
    void               Add               (ComponentMeshRenderer* object);
    void               Remove            (ComponentMeshRenderer* object);

    void               Render            (ComponentMeshRenderer* object);
    void               DoRender          (uint flags);
    void               DoUpdate          ();

    bool               HasCommands       () const { return !commands.empty();  }
    bool               IsEmpty           () const { return objects.empty(); }
    const HashString&  GetTagName        () const { return tagName; }
    ERenderMode        GetRenderMode     () const { return renderMode; }

    void               CreateRenderData  ();
    void               OnMaterialModified(UID materialID);
    ComponentMeshRenderer* FindObject(uint instanceIndex);
    unsigned            GetNumObjects() const { return unsigned(objects.size()); }

    void                DoFrustumCulling(BatchDrawCommands& drawCommands, const float4* planes, const float3& cameraPos);
    void                DoRenderCommands(const BatchDrawCommands& drawCommands);
private:

    void ClearRenderData      ();

    void CreateVertexBuffers  ();
    void CreateSkinningBuffers();
    void CreateMaterialBuffer ();
    void CreateInstanceBuffer ();
    void CreateMorphBuffer    ();
    void CreateTransformBuffer();
    void CreateCommandBuffer();
    void UpdateModels();
    void UpdateSkinning();
};

#endif /* _GEOMETRY_BATCH_H_ */
