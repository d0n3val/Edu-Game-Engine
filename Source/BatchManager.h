#ifndef _BATCHMANAGER_H_
#define _BATCHMANAGER_H_

#include "RenderList.h"
#include "HashString.h"
#include "OGL.h"

#include <vector>
#include <memory>

class GeometryBatch;
class ComponentMeshRenderer;
class BatchDrawCommands;

enum BATCH_RENDER_FLAGS
{
    BR_FLAG_KEEP_ORDER = 1 << 0,
};

struct BatchPrograms
{
    std::unique_ptr<Program> skinningProgram;
    std::unique_ptr<Program> skinningProgramNoTangents;
    std::unique_ptr<Program> culling;
    std::unique_ptr<Program> cullingTransparent;
    std::unique_ptr<Program> sortOdd;
    std::unique_ptr<Program> sortEven;
};

class BatchManager
{
    typedef std::vector<std::unique_ptr<GeometryBatch> > BatchPool;

    BatchPrograms programs;

    BatchPool batches;

public:
    BatchManager();
    ~BatchManager();

    uint Add(ComponentMeshRenderer* object, const HashString& tag);
    void Remove(ComponentMeshRenderer* object);

    void DoFrustumCulling(BatchDrawCommands& drawCommands, const float4* planes, const float3& cameraPos, bool opaque);
    void DoRenderCommands(const BatchDrawCommands& drawCommands);

    void DoUpdate();
    void DoRender(const NodeList& objects, uint flags);

    void FillBatchNames(std::vector<HashString>& names) const;
    void OnMaterialModified(UID materialID);
    ComponentMeshRenderer* FindComponentMeshRenderer(uint batchIndex, uint instanceIndex) const;
private:
    void CreateSkinningProgram();
    void CreateFrustumCullingProgram();
    void CreateSortProgram();
};

#endif /* _BATCHMANAGER_H_ */


